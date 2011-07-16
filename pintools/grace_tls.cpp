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

#define MEM_SIZE -1 // This will give us "all 1s" in a UINT address.

using std::deque;

// All the watchpoints for all threads are held here.
WatchPoint<ADDRINT, UINT32> *wp;

// This holds the opposite of watchpoints. This is the read-write sets
// that each thread holds-- this is used to find conflicts.
WatchPoint<ADDRINT, UINT32> *mem;

struct  thread_mem_data_t {
    // This data will not vanish when the thread exits.
    // It will be deleted only after the parent thread has stopped.
    UINT64          instructions;
    THREADID        pin_threadid;
    statistics_t    total_stats;
};

struct thread_wp_data_t
{
    //As a parent:
    INT32                       child_thread_num; // Number of active children of this thread.
    deque<thread_mem_data_t*>   child_data;
    deque<OS_THREAD_ID>         children_thread_ids;
    //As a child:
    OS_THREAD_ID                parent_threadid; //this points to its parent thread
    thread_mem_data_t*          self_mem_ptr; //points to its own data, which is stored by its parent.
    //As a thread itself:
    bool                        root; //this tells whether if the the thread is root. Well if it's the root thread then parent_thread_id would be invalid.
    bool                        has_had_children;
    bool                        has_had_siblings;
    bool                        children_skipped_kill;
    bool                        sibling_skipped_kill;
};

// This is the only way to determine whether a thread is the "root" or not.
// If thread num == 0, then the thread is the root.
INT32 thread_num = 0;

// This is where root stores it data. (Root has no parents).
thread_mem_data_t*   root_mem_data;

map<OS_THREAD_ID,thread_wp_data_t*>             thread_map;
map<OS_THREAD_ID,thread_wp_data_t*>::iterator   thread_map_iter;

UINT64 instruction_total;

#ifdef PRINT_TRACE
ofstream TraceFile;
#endif

//My own data
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
        "o", "grace_tls.out", "specify output file name");
KNOB<string> KnobTraceFile(KNOB_MODE_WRITEONCE, "pintool",
        "r", "grace_tls.trace", "specify the name for the trace file");

PIN_LOCK init_lock;

bool thread_commit_data_conflict(thread_mem_data_t* sibling_mem, thread_mem_data_t* this_mem) {
    if (mem->compare_ww(sibling_mem->pin_threadid, this_mem->pin_threadid))
        return true;
    else if (mem->compare_rw(sibling_mem->pin_threadid, this_mem->pin_threadid))
        return true;
    else if (mem->compare_wr(sibling_mem->pin_threadid, this_mem->pin_threadid))
        return true;
    return false;
}


VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    // Allocate things inside the thread init lock.
    GetLock(&init_lock, threadid+1);
    OS_THREAD_ID        this_threadid = PIN_GetTid();
    thread_wp_data_t*   this_thread = new thread_wp_data_t;
    this_thread->children_skipped_kill = false;
    this_thread->sibling_skipped_kill = false;
    this_thread->has_had_siblings = false;
    this_thread->has_had_children = false;

    this_thread->self_mem_ptr = new thread_mem_data_t;
    this_thread->self_mem_ptr->instructions = 0;
    this_thread->self_mem_ptr->pin_threadid = threadid;
    this_thread->self_mem_ptr->total_stats = wp->clear_statistics();

    if (thread_num == 0) {
        // Root, so it can't have a parent.  Because it doesn't have a parent,
        // we skip most of the setup code.
        this_thread->root = true;
        root_mem_data = this_thread->self_mem_ptr;
    }
    else {
        thread_wp_data_t*   parent_thread;
        this_thread->root = false;
        // Get the pointer to its parent thread.
        this_thread->parent_threadid = PIN_GetParentTid();
        parent_thread = thread_map[this_thread->parent_threadid];
        // Insert the memory for this thread into its parent's data in
        // the order of thread creation.
        parent_thread->child_data.push_back(this_thread->self_mem_ptr);
        // Set the parent to know that it has ever had a child.
        parent_thread->has_had_children = true;
        parent_thread->child_thread_num++;

        // Set all the siblings of this thread to know that they have a sibling.
        deque<OS_THREAD_ID>::iterator sibling_iter;
        if (parent_thread->children_thread_ids.begin() != parent_thread->children_thread_ids.end())
            this_thread->has_had_siblings = true;
        for(sibling_iter = parent_thread->children_thread_ids.begin();
                sibling_iter != parent_thread->children_thread_ids.end();
                sibling_iter++) {
            thread_map[*sibling_iter]->has_had_siblings = true;
        }
        parent_thread->children_thread_ids.push_back(this_threadid);
    }
    this_thread->child_thread_num = 0;
    
    thread_map[this_threadid] = this_thread;
    thread_num++;

    wp->start_thread(threadid);
    mem->start_thread(threadid);
    wp->set_watch(0, MEM_SIZE, threadid);

    ReleaseLock(&init_lock);
}

VOID get_local_stats(thread_mem_data_t* mem_ptr)
{
    statistics_t local_stats = wp->get_statistics(mem_ptr->pin_threadid);
    mem_ptr->total_stats += local_stats;
}

// This is used when a conflict happens and we need to add the stuff into
// the total a second time. 
VOID add_data_to_total(thread_mem_data_t* add_mem_ptr)
{
    instruction_total += add_mem_ptr->instructions;
}

// This doubles the values contained in a thread due to a conflict happening
VOID double_data(thread_mem_data_t* double_mem_ptr)
{
    statistics_t double_me;
    double_me = double_mem_ptr->total_stats;
    double_mem_ptr->total_stats += double_me;
}

VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    OS_THREAD_ID this_threadid;
    thread_wp_data_t* this_thread;
    OS_THREAD_ID this_parent_threadid;
    thread_mem_data_t* child_mem_ptr;
    thread_mem_data_t* parent_mem_ptr;
    thread_mem_data_t* compare_mem_ptr;
    GetLock(&init_lock, threadid+1);

    this_threadid = PIN_GetTid();
    this_thread = thread_map[this_threadid];
    this_parent_threadid = this_thread->parent_threadid;

    get_local_stats(this_thread->self_mem_ptr);

    if (!this_thread->root) { // Never kill off the root thread
        // This thread is done, so remove one of the parente thread's children.
        thread_map[this_parent_threadid]->child_thread_num--;

        if (thread_map[this_parent_threadid]->child_thread_num == 0) {
            // If all the CURRENT children for the parent thread are now done, we need to run grace comparison against them.
            deque<thread_mem_data_t*>::iterator child_iter;
            deque<thread_mem_data_t*>::iterator compare_iter;

            parent_mem_ptr = thread_map[this_parent_threadid]->self_mem_ptr;

            for (child_iter = (thread_map[this_parent_threadid]->child_data).end() -1;
                    child_iter != (thread_map[this_parent_threadid]->child_data).begin();
                    child_iter--) {
                bool did_conflict = false;
                child_mem_ptr = *child_iter;
                // The total number of trie/range misses sets etc the system
                // sees is at least how many happened in each child thread.
                add_data_to_total(child_mem_ptr);
                for (compare_iter = (thread_map[this_parent_threadid]->child_data).begin();
                        compare_iter != child_iter;
                        compare_iter++) {
                    compare_mem_ptr = *compare_iter;
                    if (thread_commit_data_conflict(compare_mem_ptr, child_mem_ptr) ) {
                        // There was a conflict between a thread and an earlier sibling thread.
                        // Therefore, the second thread had to run twice.
                        did_conflict = true;
                        add_data_to_total(child_mem_ptr);
                        double_data(child_mem_ptr);
                        break;
                    }
                }
                if (!did_conflict && thread_commit_data_conflict(parent_mem_ptr, child_mem_ptr) ) {
                    // Didn't conflict with any other child thread, but conflicted with the parent.
                    add_data_to_total(child_mem_ptr);
                    double_data(child_mem_ptr);
                }
                parent_mem_ptr->total_stats += child_mem_ptr->total_stats;
            }
            child_mem_ptr = *child_iter; // Must also check thread at begin()
            add_data_to_total(child_mem_ptr);
            if (thread_commit_data_conflict(parent_mem_ptr, child_mem_ptr) ) {
                add_data_to_total(child_mem_ptr);
                double_data(child_mem_ptr);
            }
            parent_mem_ptr->total_stats += child_mem_ptr->total_stats;
            // The parent will either be handled when IT dies, or will be handled by
            // the else staement below if it's the root.
        }
    }
    else {
        // Can't have conflict if this is a root thread.
        add_data_to_total(this_thread->self_mem_ptr);
    }

    // Delete stuff now.
    if (!this_thread->root) {
        if (thread_map[this_parent_threadid]->child_thread_num == 0) {
            // Thread information cleanup.
            if (!this_thread->has_had_siblings && !this_thread->has_had_children) {
                // If this guy had no siblings or children, no one will clean him up.
                // He must do it himself.
                delete this_thread->self_mem_ptr;
                delete thread_map[this_threadid];
                thread_map.erase(this_threadid);
                thread_num--;
            }
            else {
                // This thread has either siblings or children.  We're in here because the parent
                // has no more live children, so we should first try to delete all siblings.
                deque<OS_THREAD_ID>::iterator sibling_thread_id;
                // Walking through the siblings will also catch us.
                for(sibling_thread_id = (thread_map[this_parent_threadid]->children_thread_ids).begin();
                        sibling_thread_id != (thread_map[this_parent_threadid]->children_thread_ids).end();
                        sibling_thread_id++) {
                    thread_wp_data_t *sibling_thread = thread_map[*sibling_thread_id];
                    if (!thread_map[this_parent_threadid]->child_thread_num) {
                        // If the sibling never had children it is absolutely OK to delete its data.
                        // If the sibling HAD children but the number is at zero, then there is
                        // no one left to delete it, so we must.
                        if (!sibling_thread->has_had_children || !sibling_thread->child_thread_num){
                            delete sibling_thread->self_mem_ptr;
                            delete thread_map[*sibling_thread_id];
                            thread_map.erase(*sibling_thread_id);
                            thread_num--;
                        }
                        else {
                            // If this sibling thread still has live children, then THEY
                            // must delete it.
                            // We'll leave it around for its last child to delete.
                            sibling_thread->sibling_skipped_kill = true;
                        }
                    }
                }
            }
            (thread_map[this_parent_threadid]->child_data).clear();
            (thread_map[this_parent_threadid]->children_thread_ids).clear();

            if (thread_map[this_parent_threadid]->sibling_skipped_kill) {
                // If one of our parents siblings skipped killing it because we (the child) existed
                // We, the last child, must be the one to kill it.
                // (This also includes the one parent one child case, where it skips killing itself in the sibling-list walk).
                thread_wp_data_t *parent_thread = thread_map[this_parent_threadid];;
                delete parent_thread->self_mem_ptr;
                delete thread_map[this_parent_threadid];
                thread_map.erase(this_parent_threadid);
                thread_num--;
            }
        }
    }

    wp->end_thread(threadid);
    mem->end_thread(threadid);

    ReleaseLock(&init_lock);
}

// This would check for read watchfault. And save it to mem(as read set) if there are any.
VOID RecordMemRead(VOID * ip, ADDRINT addr, UINT32 size, THREADID threadid)
{
    GetLock(&init_lock, threadid+1);
    // If more than one thread is running, we need to be using Grace.
    if (thread_num > 1) {
        // Check if this thread read-owns this location. This is a real check.
        // This entire thing must be globally locked, or we could enter a state where
        // another thread updates the oracle while we walk the internals.
        if ( wp->read_fault(addr, (ADDRINT)(addr+size-1), threadid) ) {
            // If so, remove the read watchpoint from this address and set it in the read set.
            wp->rm_read(addr, (ADDRINT)(addr+size-1), threadid);
            mem->update_set_read(addr, (ADDRINT)(addr+size-1), threadid, IGNORE_STATS);
        }
    }
    ReleaseLock(&init_lock);
    return;
}

// This would check for write watchfault. And save it to mem(as read set) if there are any.
VOID RecordMemWrite(VOID * ip, ADDRINT addr, UINT32 size, THREADID threadid)
{
    GetLock(&init_lock, threadid+1);
    if (thread_num > 1) {
        if ( wp->write_fault(addr, (ADDRINT)(addr+size-1), threadid) ) {
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
VOID PIN_FAST_ANALYSIS_CALL docount(ADDRINT c)
{
    thread_map[PIN_GetTid()]->self_mem_ptr->instructions += c;
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
                BBL_NumIns(bbl), IARG_END);
    }
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{
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
    GetLock(&init_lock, 0xdeadface);
    // Write to a file since cout and cerr maybe closed by the application
    ofstream OutFile;
    OutFile.open(KnobOutputFile.Value().c_str());
    OutFile << "Total number of instructions: " << instruction_total << endl;

    OutFile << "Total Statistics for Grace System: " << endl;
    wp->print_statistics(root_mem_data->total_stats, OutFile);
    ReleaseLock(&init_lock);

    OutFile.close();
}

VOID DataInit() {
#ifdef PRINT_TRACE
    total_print_number = 0;
    TraceFile.open(KnobTraceFile.Value().c_str());
    wp = new WatchPoint<ADDRINT, UINT32>(TraceFile);
#else
    wp = new WatchPoint<ADDRINT, UINT32>;
#endif
    mem = new WatchPoint<ADDRINT, UINT32>(false);
    instruction_total = 0;
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "Grace Watchpoint system." << endl;
    cerr << "  Just give this guy a fork/join program to run." << endl;
    cerr << "NOTE: This will not work with LibGomp OpenMP programs ";
    cerr << "because they don't kill the worker threads at the end of ";
    cerr << "each parallel region.  Use grace_omp.so for them." << endl;
    cerr << "Will give output data in grace_tls.out unless you give ";
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
