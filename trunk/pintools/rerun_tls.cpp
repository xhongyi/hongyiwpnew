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
#include <cstring>

//#define RERUN

#ifndef RERUN
#define UNSOUND_RACE_DETECTOR
#endif

#include <map>
#include <iostream>
#include <fstream>
#include "pin.H"
#include "watchpoint.h"

// Force each thread's data to be in its own data cache line so that
// multiple threads do not contend for the same data cache line.
#define PADSIZE 56  // 64 byte line size
                    // 'number_of_instructions' is 8 bytes, so pad takes
                    // 64-8=56 bytes on a 32-bit compile.
#define MEM_SIZE -1 // this will give us "all 1s" in a UINT address.

using std::deque;

//This will hold statistics for each individual thread.
struct thread_wp_data_t
{
    UINT64 number_of_instructions;
    UINT8 pad[PADSIZE];
};

static TLS_KEY tls_key;

// All the watchpoints for all threads are held here.
WatchPoint<ADDRINT, UINT32> *wp;

// This holds the opposite of watchpoints. This is the read-write sets
// that each thread holds-- this is used to find conflicts.
WatchPoint<ADDRINT, UINT32> *mem;

map<THREADID,thread_wp_data_t*> thread_map;

deque<THREADID> all_threads;
deque<THREADID> live_threads;

// Statistics for the program as a whole.
UINT64 instruction_total;
statistics_t all_threads_stats;

#if defined(PRINT_TRACE)
ofstream TraceFile;
#endif

//My own data
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
        "o", "rerun_tls.out", "specify output file name");
KNOB<string> KnobTraceFile(KNOB_MODE_WRITEONCE, "pintool",
        "r", "rerun_tls.trace", "specify the name for the trace file");

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

    // Initialization says that this thread has run no instructions
    this_thread->number_of_instructions = 0;

    // Keep track of which threads are around (so we can update statistics)
    thread_map[threadid] = this_thread;
    // Use your thread local data only to access the pointer to your own structure.
    PIN_SetThreadData(tls_key, this_thread, threadid);

    // Start this thread and set its watchpoints everywhere.
    wp->start_thread(threadid);
    mem->start_thread(threadid);
    wp->set_watch(0, MEM_SIZE, threadid);

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
    all_threads_stats += wp->get_statistics(threadid);
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
    wp->end_thread(threadid);
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
    
    // If more than one thread is running, we need to turn on the race detector.
    GetLock(&init_lock, threadid+1);
    if (live_threads.size() > 1) {
        // Check if this thread read-owns this location. This is a real check.
        // This must be entirely locked, or we could enter a disgusting state when walking
        // the oracle internals and another thread updates us.
        if ( wp->read_fault(addr, (ADDRINT)(addr+size-1), threadid) ) {
            /* Took a read fault. We must now see if this location was in the write-set of
             * someone else. If so, we need to remove their ownership and watchpoint. */
            for(live_iter = live_threads.begin(); live_iter != live_threads.end(); live_iter++) {
                if (*live_iter != threadid) {
                    if (mem->write_fault(addr, (ADDRINT)(addr+size-1), *live_iter) ) {
#ifdef RERUN
                       // In RERUN this causes an episode to end.
                       // We store off the read/write sets of this thread before
                       // the conflict for offline data-race detection.
                       
                       // Rerun's logic states that if Thread 1 conflicts with Thread 2,
                       // Thread 2 must end its episode.
                       wp->set_watch(0, MEM_SIZE, *live_iter, STORE_STATS);
                       mem->rm_watch(0, MEM_SIZE, *live_iter, IGNORE_STATS);
#else
                       // In ONLINE DATA RACE DETECTION, this should just turn
                       // on the race detector and then return with the read/write
                       // ownerships changed.
                       // Only synchronization events should cause stuff to clear out
                       // Remove from the write set of the remote thread.
                       wp->update_set_write(addr, (ADDRINT)(addr+size-1), *live_iter);
                       mem->rm_write(addr, (ADDRINT)(addr+size-1), *live_iter, IGNORE_STATS);
#endif
                       // Two threads could have this read in their write sets if this
                       // overlaps with two different ranges. Keep on trucking through
                       // this for-loop. Even though no two threads will have an individual
                       // byte in both their write-sets, our range could be in two things.
                    }
                }
            }

            // Add to our local read set.
            wp->rm_read(addr, (ADDRINT)(addr+size-1), threadid);
            mem->update_set_read(addr, (ADDRINT)(addr+size-1), threadid, IGNORE_STATS);
        }
    }
    ReleaseLock(&init_lock);
    return;
}

// This thread is attempting a memory write.
VOID RecordMemWrite(VOID * ip, ADDRINT addr, UINT32 size, THREADID threadid)
{
    deque<THREADID>::iterator live_iter;

    // If more than one thread is running, we need to turn on the race detector.
    GetLock(&init_lock, threadid+1);
    if (live_threads.size() > 1) {
        // Check if this thread write-owns this location. This is a real check.
        // This must be entirely locked, or we could enter a disgusting state when walking
        // the oracle internals and another thread updates us.
        if ( wp->write_fault(addr, (ADDRINT)(addr+size-1), threadid) ) {
            /* Took a write fault. We must now see if this location was in the read or write sets of
             * someone else. If so, we need to remove their ownership and watchpoint. */
            for(live_iter = live_threads.begin(); live_iter != live_threads.end(); live_iter++) {
                if (*live_iter != threadid) {
                    if (mem->watch_fault(addr, (ADDRINT)(addr+size-1), *live_iter) ) {
#ifdef RERUN
                       // In RERUN this causes an episode to end.
                       // We store off the read/write sets of this thread before
                       // the conflict for offline data-race detection.

                       // Rerun's logic states that if Thread 1 conflicts with Thread 2,
                       // Thread 2 must end its episode.
                       wp->set_watch(0, MEM_SIZE, *live_iter, STORE_STATS);
                       mem->rm_watch(0, MEM_SIZE, *live_iter, IGNORE_STATS);
#else
                       // In ONLINE DATA RACE DETECTION, this should just turn
                       // on the race detector and then return with the read/write
                       // ownerships changed.
                       // Only synchronization events should cause stuff to clear out
                       // Remove access from the remove thread. We own this puppy now.
                       wp->set_watch(addr, (ADDRINT)(addr+size-1), *live_iter);
                       mem->rm_watch(addr, (ADDRINT)(addr+size-1), *live_iter, IGNORE_STATS);
#endif
                       // Two threads could have this read in their write sets if this
                       // overlaps with two different ranges. Keep on trucking through
                       // this for-loop. Even though no two threads will have an individual
                       // byte in both their write-sets, our range could be in two things.
                    }
                }
            }

            // Technically, you don't need to pay attention to any other reads now either,
            // because you now write-own this location. Any other access on the machine, even reads,
            // to this location will cause a write-sharing event.
            wp->rm_watch(addr, (ADDRINT)(addr+size-1), threadid);
            mem->update_set_write(addr, (ADDRINT)(addr+size-1), threadid, IGNORE_STATS);
        }
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

typedef struct _FUNCTION_NAMES {
        const char *name;
} FUNCTION_NAMES;
typedef const FUNCTION_NAMES *FUNCTION_NAME_ARRAY;

// Only LOCK instructions (which increase our local clock) cause us to remove
// our own watchpoints. This way, anything we touch has its local clock
// incremented at the very least.
static const FUNCTION_NAMES __tcPthreadsPPCEPs[] = {
    //{"pthread_mutex_init"},
    //{"pthread_mutex_destroy"},
    {"pthread_mutex_lock"},
    //{"pthread_mutex_unlock"},
    {"pthread_mutex_trylock"},
    //{"pthread_spin_init"},
    //{"pthread_spin_destroy"},
    {"pthread_spin_lock"},
    {"pthread_spin_trylock"},
    //{"pthread_spin_unlock"},

    //{"pthread_cond_init"},
    //{"pthread_cond_destroy"},
    //{"pthread_cond_broadcast"},
    //{"pthread_cond_signal"},
    {"pthread_cond_wait"},
    {"pthread_cond_timedwait"},

    //{"pthread_rwlock_init"},
    //{"pthread_rwlock_destroy"},
    {"pthread_rwlock_rdlock"},
    {"pthread_rwlock_tryrdlock"},
    {"pthread_rwlock_wrlock"},
    {"pthread_rwlock_trywrlock"},
    //{"pthread_rwlock_unlock"},

    //{"pthread_barrier_init"},
    //{"pthread_barrier_destroy"},
    {"pthread_barrier_wait"},
    { NULL}
};

void SynchronizationCall(THREADID threadid)
{
    GetLock(&init_lock, threadid+1);
    wp->set_watch(0, MEM_SIZE, threadid, STORE_STATS);
    mem->rm_watch(0, MEM_SIZE, threadid, IGNORE_STATS);
    ReleaseLock(&init_lock);
}

void InstrumentFunction(RTN rtn, const char *symname, const char *imgname)
{
    RTN_Open(rtn);
    //cerr << "Instrumenting function " << symname << " in library " << imgname << endl;
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)SynchronizationCall, IARG_THREAD_ID, IARG_END);
    RTN_Close(rtn);
}

void InstrumentFunctions(IMG img, const char *name, const FUNCTION_NAME_ARRAY *functions)
{
    unsigned long loadoffset;
    void *faddr;
    SYM sym;
    RTN rtn;
    string local_name_copy;
    const FUNCTION_NAME_ARRAY *fnc;

    loadoffset = IMG_LoadOffset(img);
    for (sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym)) {
        if ((faddr = (void *)(SYM_Value(sym) + loadoffset)) == 0)
            continue;
        rtn = RTN_FindByAddress((ADDRINT)faddr);
        local_name_copy = SYM_Name(sym);
        // Check if function name matches the ptrheads stuff we care about.
        replace(local_name_copy.begin(), local_name_copy.end(), '@', '\0');

        // Probably only pthreads (unless we somehow link it twice), but let's look through the
        // array anyway.
        for (fnc = functions; *fnc != NULL; fnc++) {
            const FUNCTION_NAMES *p;
            // Now to loop through the function names and instrument the ones in our list up there.
            for (p = *fnc; p->name != NULL; p++) {
                if ((strcmp(PIN_UndecorateSymbolName(local_name_copy, UNDECORATION_NAME_ONLY).c_str(), p->name) == 0) ||
                        (strcmp(local_name_copy.c_str(), p->name) == 0)) {
                    InstrumentFunction(rtn, local_name_copy.c_str(), name);
                    break;
                }
            }
        }
    }
}

FUNCTION_NAME_ARRAY* FindPthreads(char *name, FUNCTION_NAME_ARRAY *functions)
{
    FUNCTION_NAME_ARRAY *f = functions;
    *f = (FUNCTION_NAMES *)NULL;
    if (!strcmp("/lib/libpthread.so.0", name)) {
        *f++ = __tcPthreadsPPCEPs;
    }
    *f = NULL;
    return ((f == functions) ? NULL : functions);
}

void ImageLoad(IMG img)
{
    char *name;
    const FUNCTION_NAMES *function_table[10];

    name = (char *)(IMG_Name(img).c_str());
    if(FindPthreads(name, function_table) != 0) {
        InstrumentFunctions(img, name, function_table);
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
    while(!all_threads.empty()) {
        thread_num = all_threads.front();
        all_threads.pop_front();
        OutFile << "\tThread " << thread_num << " instruction count: ";
        OutFile << thread_map[thread_num]->number_of_instructions << endl;
        delete thread_map[thread_num];
        thread_map.erase(thread_num);
    }
    OutFile << endl << "==============================" << endl << endl;

    wp->print_statistics(OutFile, INCLUDE_INACTIVE);
    ReleaseLock(&init_lock);
    
    OutFile.close();
}

VOID DataInit() {
#ifdef PRINT_TRACE
    TraceFile.open(KnobTraceFile.Value().c_str());
    wp = new WatchPoint<ADDRINT, UINT32>(TraceFile);
#else
    wp = new WatchPoint<ADDRINT, UINT32>();
#endif
    mem = new WatchPoint<ADDRINT, UINT32>(false);
    instruction_total = 0;
    all_threads_stats = wp->clear_statistics();
    return;
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "Rerun Watchpoint system." << endl;
    cerr << "  Just give this a parallel program to run." << endl;
    cerr << "Will give output data in rerun_tls.out unless you give ";
    cerr << "it a -o {name} option." << endl;
    cerr << "Will potentially give a trace of operations to the backing store ";
    cerr << "into a file. Use -r {name} to change this location." << endl;
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
#ifndef UNSOUND_RACE_DETECTOR
    IMG_AddInstrumentFunction((IMAGECALLBACK)ImageLoad, 0);
#endif

    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Instruction to be called to instrument instructions.
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits.
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
