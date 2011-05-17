/** Copyright 2010 University of Michigan

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. **/

#ifndef vector_H
#include <deque>
#define vector_H
#endif

#include <map>
#include <iostream>
#include <fstream>
#include "pin.H"
#include "watchpoint.h"

// Force each thread's data to be in its own data cache line so that
// multiple threads do not contend for the same data cache line.
#define PADSIZE 36  // 64 byte line size
                    // 'number_of_instructions', 'number_of_memops', and
                    // 'number_of_shared_memops' are 8 bytes each.
                    // pointer to the thread local lock is 4 bytes.
                    // 64-8-8-8-4=36 bytes on a 32-bit compile.
#define MEM_SIZE -1 // this will give us "all 1s" in a UINT address.

using std::deque;

//This will hold statistics for each individual thread.
struct thread_wp_data_t
{
    UINT64 number_of_instructions;
    UINT64 number_of_memops;
    UINT64 number_of_shared_memops;
    PIN_LOCK *thread_local_lock;
    UINT8 pad[PADSIZE];
};

static TLS_KEY tls_key;

// This is the read-write sets that each thread holds-- this is used to
// find and record conflicts.
WatchPoint<ADDRINT, UINT32> *mem;

map<THREADID,thread_wp_data_t*> thread_map;

deque<THREADID> all_threads;
deque<THREADID> live_threads;

// Statistics for the program as a whole.
UINT64 instruction_total;
UINT64 memop_total;
UINT64 shared_memop_total;

//My own data
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "sharing_detector.out", "specify output file name");

// This is "the big lock". You must hold this if you're going to
// touch the thread map. However, if you're just going to grab
// the locks held by the thread local stuff pointed to by the
// thread map, then you can use the PIN thread local storage stuff.
PIN_LOCK init_lock;

VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    // Allocate things inside the thread init lock.
    GetLock(&init_lock, threadid+1);
    thread_wp_data_t* this_thread = new thread_wp_data_t;
    this_thread->thread_local_lock = new PIN_LOCK;
    InitLock(this_thread->thread_local_lock);

    // Initialization says that this thread has run no instructions
    this_thread->number_of_instructions = 0;
    this_thread->number_of_memops = 0;
    this_thread->number_of_shared_memops = 0;

    // Keep track of which threads are around (so we can update statistics)
    thread_map[threadid] = this_thread;
    // Use your thread local data only to access the pointer to your own structure.
    PIN_SetThreadData(tls_key, this_thread, threadid);

    // Start this thread and set its watchpoints everywhere.
    mem->start_thread(threadid);

    // Keep track of live threads so that we can compare read/write sets with
    // all other live threads on reads + writes.
    live_threads.push_back(threadid);
    all_threads.push_back(threadid);

    ReleaseLock(&init_lock);
}

// Must be called while holding init_lock
VOID AddThreadDataToTotal(THREADID threadid)
{
    instruction_total += (thread_map[threadid])->number_of_instructions;
    memop_total += (thread_map[threadid])->number_of_memops;
    shared_memop_total += (thread_map[threadid])->number_of_shared_memops;
}

// Thread is done. Remove it from the watchpoint system, add its statistics
// to the global total.
// Move this previously-live thread to the dead-thread list.
// Keep the thread_map information around so we can print thing at the end.
VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    deque<THREADID>::iterator iter;
    GetLock(&init_lock, threadid+1);
    AddThreadDataToTotal(threadid);
    mem->end_thread(threadid);
    for(iter = live_threads.begin(); iter != live_threads.end(); iter++) {
        if(*iter == threadid) {
            live_threads.erase(iter);
            break;
        }
    }
    ReleaseLock(&init_lock);
}

// This thread is attempting a memory read.
VOID RecordMemRead(VOID * ip, ADDRINT addr, UINT32 size, THREADID threadid)
{
    deque<THREADID>::iterator live_iter;
    thread_wp_data_t* this_thread;

    // If more than one thread is running, we need to turn check for data sharing.
    GetLock(&init_lock, threadid+1);
    this_thread = thread_map[threadid];
    this_thread->number_of_memops += 1;

    if (live_threads.size() > 1) {
        /* We must see if this location was in the write-set of someone else. If so, 
         * there was R->W sharing, and we need to remove their ownership. */
        for(live_iter = live_threads.begin(); live_iter != live_threads.end(); live_iter++) {
            if (*live_iter != threadid) {
                if (mem->write_fault(addr, (ADDRINT)(addr+size-1), *live_iter, IGNORE_STATS) ) {
                    // There was another thread that has written to this location previously
                    // and we just tried to read it. R/W data sharing.
                    this_thread->number_of_shared_memops += 1;

                    // We need to remove this location from "everyone else". However,
                    // the W->W data share detector will make sure that only one thread can
                    // be write-shared at a time. This will remove it.
                    mem->rm_watch(addr, (ADDRINT)(addr+size-1), *live_iter, IGNORE_STATS);
                    break;
                }
            }
        }
        // Now it's in our read set.
        mem->update_set_read(addr, (ADDRINT)(addr+size-1), threadid, IGNORE_STATS);
    }
    ReleaseLock(&init_lock);
    return;
}

// This thread is attempting a memory write.
VOID RecordMemWrite(VOID * ip, ADDRINT addr, UINT32 size, THREADID threadid)
{
    deque<THREADID>::iterator live_iter;
    deque<THREADID>::iterator killing_iter;
    thread_wp_data_t* this_thread;

    // If more than one thread is running, we need to turn on the race detector.
    GetLock(&init_lock, threadid+1);
    this_thread = thread_map[threadid];
    this_thread->number_of_memops += 1;

    if (live_threads.size() > 1) {
        /* We must see if this location was in the read or write sets of someone else. If so,
         * there was data sharing and we need to remove their ownership. */
        for(live_iter = live_threads.begin(); live_iter != live_threads.end(); live_iter++) {
            if (*live_iter != threadid) {
                if (mem->watch_fault(addr, (ADDRINT)(addr+size-1), *live_iter, IGNORE_STATS) ) {
                    // There was another thread that has touched to this location previously
                    // and we just tried to write to it.
                    this_thread->number_of_shared_memops += 1;

                    // We need to remove this location from everyone else.
                    for(killing_iter = live_iter; killing_iter != live_threads.end(); killing_iter++) {
                        mem->rm_watch(addr, (ADDRINT)(addr+size-1), *killing_iter, IGNORE_STATS);
                    }
                    break;
                }
            }
        }

        mem->update_set_write((ADDRINT)addr, (ADDRINT)(addr+size-1), threadid, IGNORE_STATS);
    }
    ReleaseLock(&init_lock);
    return;
}

// This function is called before every block
VOID PIN_FAST_ANALYSIS_CALL docount(ADDRINT c, THREADID tid)
{
    (static_cast<thread_wp_data_t *>(PIN_GetThreadData(tls_key, tid)))->number_of_instructions += c;
}

// Count the number of each specific instruction.
VOID Trace(TRACE trace, VOID *v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to docount for every bbl, passing the number of instructions.
        // IPOINT_ANYWHERE allows Pin to schedule the call anywhere in the bbl to obtain best performance.
        BBL_InsertCall(bbl, IPOINT_ANYWHERE, (AFUNPTR)docount, IARG_FAST_ANALYSIS_CALL, IARG_UINT32, 
                BBL_NumIns(bbl), IARG_THREAD_ID, IARG_END);
    }
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{
    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // The IA-64 architecture has explicitly predicated instructions. 
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
    // prefixed instructions appear as predicated instructions in Pin.
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_MEMORYREAD_SIZE,
                IARG_THREAD_ID,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertCall  (
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_MEMORYWRITE_SIZE,
                IARG_THREAD_ID,
                IARG_END);
        }
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    THREADID thread_num;
    deque<THREADID>::iterator live_iter;
    GetLock(&init_lock, 0xdeadface);
    for(live_iter = live_threads.begin(); live_iter != live_threads.end(); live_iter++) {
        AddThreadDataToTotal(*live_iter);
    }
    ofstream OutFile;
    OutFile.open(KnobOutputFile.Value().c_str());
    // Write to a file since cout and cerr maybe closed by the application
    OutFile << "Total number of instructions: " << instruction_total << endl;
    OutFile << "Total memory ops: " << memop_total << endl;
    OutFile << "Total shared memory ops: " << shared_memop_total << endl;
    while(!all_threads.empty()) {
        thread_num = all_threads.front();
        all_threads.pop_front();
        OutFile << "\tThread " << thread_num << " instruction count: ";
        OutFile << thread_map[thread_num]->number_of_instructions << endl;
        OutFile << "\tThread " << thread_num << " memop count: ";
        OutFile << thread_map[thread_num]->number_of_memops << endl;
        OutFile << "\tThread " << thread_num << " shared memop count: ";
        OutFile << thread_map[thread_num]->number_of_shared_memops << endl << endl;
        delete thread_map[thread_num];
        thread_map.erase(thread_num);
    }
    OutFile <<  "==============================" << endl << endl;
    ReleaseLock(&init_lock);
    
////////////////////////Out put the data collected

    OutFile.close();
}

VOID DataInit() {
    mem = new WatchPoint<ADDRINT, UINT32>(false);
    instruction_total = 0;
    memop_total = 0;
    shared_memop_total = 0;
    return;
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "Sharing detection system." << endl;
    cerr << "  Just give this a parallel program to run." << endl;
    cerr << "Will give output data in sharing_detection.out unless you give ";
    cerr << "it a -o {name} option." << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return Usage();

    // Initialize the init_lock
    InitLock(&init_lock);

    tls_key = PIN_CreateThreadDataKey(0);
    
    DataInit();

    // Register ThreadStart to be called when a thread starts.
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);

    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Instruction to be called to instrument instructions.
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits.
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
