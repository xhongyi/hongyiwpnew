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

#include <iostream>
#include <fstream>
#include <cstdlib>
#include "pin.H"

#define NUM_PROCS 4

#define PADSIZE 52 // 64 byte cache line size
                   // 'number_of_instructions' is 8 bytes
                   // 'processors' is 4 bytes
                   // So we need 52 bytes of padding.

// This holds the local data for each thread.
// Most important part is which processor this thread is assigned to.
struct thread_data_t
{
    UINT64 number_of_instructions;
    unsigned int processor;
    UINT8 pad[PADSIZE];
};

static TLS_KEY tls_key;

deque<THREADID> live_threads;


// Keep track of the number of instructions we've executed since starting
// multithreaded code.
UINT64 mt_instruction_total;

//My own data
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "rerun_ins.out", "specify output file name");

KNOB<UINT64> KnobStartInstruction(KNOB_MODE_WRITEONCE, "pintool",
        "s", "100000000", "specify the first instruction to start printing");
KNOB<UINT64> KnobPrintCount(KNOB_MODE_WRITEONCE, "pintool",
        "n", "10000000", "specify the number of instructions to print");

UINT64 instruction_start_count;
UINT64 instruction_end_count;

// This is "the big lock". You must hold this if you're going to
// touch the thread map. However, if you're just going to grab
// the locks held by the thread local stuff pointed to by the
// thread map, then you can use the PIN thread local storage stuff.
PIN_LOCK init_lock;

// We're going to have to output stuff all the dang time.
ofstream OutFile;

VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    unsigned int procs_in_use[NUM_PROCS];
    unsigned int which_to_use;
    deque<THREADID>::iterator iter;

    GetLock(&init_lock, threadid+1);
    thread_data_t* this_thread = new thread_data_t;
    this_thread->number_of_instructions = 0;
    // Set the processor for this thread by checking everyone else.
    // Find the least-used one.
    for(int i = 0; i < NUM_PROCS; i++)
        procs_in_use[i] = 0 ;
    for(iter = live_threads.begin(); iter != live_threads.end(); iter++) {
        unsigned int active_core;
        active_core = (static_cast<thread_data_t *>(PIN_GetThreadData(tls_key, *iter)))->processor;
        procs_in_use[active_core] += 1;
    }
    which_to_use = 0;
    for(int i = 1; i < NUM_PROCS; i++) {
        if(procs_in_use[i] < procs_in_use[which_to_use])
            which_to_use = i;
    }
    cerr << "Launching thread " << threadid << ". Putting it on core: ";
    cerr << which_to_use << endl;
    this_thread->processor = which_to_use;

    // Add this thread to the thread-local-store thing. This way when we're printing, we can know our core easily.
    PIN_SetThreadData(tls_key, this_thread, threadid);
    live_threads.push_back(threadid);
    ReleaseLock(&init_lock);
}

VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    deque<THREADID>::iterator iter;
    GetLock(&init_lock, threadid+1);
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
    GetLock(&init_lock, threadid+1);
    if(mt_instruction_total > instruction_start_count) {
        OutFile << (static_cast<thread_data_t *>(PIN_GetThreadData(tls_key, threadid)))->processor << " 0x";
        OutFile << hex << addr;
        OutFile << " 0 " << "LD" << endl;
    }
    ReleaseLock(&init_lock);
    return;
}

// This thread is attempting a memory write.
VOID RecordMemWrite(VOID * ip, ADDRINT addr, UINT32 size, THREADID threadid)
{
    GetLock(&init_lock, threadid+1);
    if(mt_instruction_total > instruction_start_count) {
        OutFile << (static_cast<thread_data_t *>(PIN_GetThreadData(tls_key, threadid)))->processor  << " 0x";
        OutFile << hex << addr;
        OutFile << " 0 " << "ST" << endl;
    }
    ReleaseLock(&init_lock);
    return;
}

// This function is called before every block
VOID PIN_FAST_ANALYSIS_CALL docount(ADDRINT c, THREADID tid)
{
    GetLock(&init_lock, tid+1);
    mt_instruction_total += c;
    if(mt_instruction_total > instruction_end_count)
        exit(0);
    ReleaseLock(&init_lock);
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
    OutFile.close();
}

VOID DataInit() {
    mt_instruction_total = 0;
    instruction_start_count = KnobStartInstruction.Value();
    instruction_end_count = instruction_start_count + KnobPrintCount.Value();
    OutFile.open(KnobOutputFile.Value().c_str());
    return;
}

BOOL InterruptQuit(THREADID tid, INT32 sig, CONTEXT *ctxt, BOOL hndlr, const EXCEPTION_INFO *excpt, VOID *useless) {
    Fini(0, 0);
    exit(0);
    return true;
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "Rerun memory printing system." << endl;
    cerr << "  Give this a parallel program to run." << endl;
    cerr << "Will output the instructions to rerun_ins.out unless you give ";
    cerr << "it a -o {name} option." << endl;
    cerr << "Use -s {instruction number} to start keeping track of instrucions after reaching ";
    cerr << "this many total executed instructions. Default is 100,000,000" << endl;
    cerr << "Use -n {instruction count} to set the number of instructions to print ";
    cerr << "before the program quits. Default is 10,000,000" << endl;
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

    DataInit();

    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    PIN_InterceptSignal(2, InterruptQuit, NULL);

    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Instruction to be called to instrument instructions.
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits.
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
