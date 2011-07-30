#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#define NOT_WATCHED     0
#define WRITE_WATCHED   1
#define READ_WATCHED    2
#define RW_WATCHED      3

using namespace std;

// Opcodes:
//      0 - Set 00
//      1 - Set 01
//      2 - Set 10
//      3 - Set 11
//      4 - Update 1x
//      5 - Update x1
//      6 - Update 0x
//      7 - Update x0
//      8 - Check address range.
//      9 - Also check the addr range
//      10 - print this thread
struct pin_command {
    unsigned int start_addr;
    unsigned int end_addr;
    unsigned short thread_id;
    short opcode;
};

int max_thread;

map<unsigned int, unsigned int> *thread_to_rc;

vector<pin_command> pin_commands;

void print_thing(map<unsigned int, unsigned int> &range_cache)
{
    map<unsigned int, unsigned int>::iterator range_cache_iter;
    cout << "Printing all ranges: " << endl;
    for(range_cache_iter = range_cache.begin();
            range_cache_iter != range_cache.end(); range_cache_iter++) {
        cout << hex << range_cache_iter->first << ": " << range_cache_iter->second << endl;
    }
}

bool check_range(map<unsigned int, unsigned int> &range_cache, unsigned int start_addr, unsigned int end_addr, int flag)
{
    map<unsigned int, unsigned int>::iterator iter_low, iter_high;
    iter_low = range_cache.upper_bound(start_addr);
    iter_low--;
    iter_high = range_cache.upper_bound(end_addr);
    iter_high--;
    return((iter_low->second | flag) || (iter_high->second | flag));
}

void add_range(map<unsigned int, unsigned int> &range_cache, unsigned int start_addr, unsigned int end_addr, int flag)
{
    map<unsigned int, unsigned int>::iterator iter_low, iter_high;
    unsigned int first_addr = 0;
    unsigned int last_addr = -1;
    int first_flags, last_flags;
    int super_previous_flags, super_later_flags;
    unsigned int super_previous_addr = 0;
    unsigned int super_later_addr = 0;

    // Find the "range that includes our start point"
    iter_low = range_cache.upper_bound(start_addr);
    iter_low--;
    first_flags = iter_low->second;
    first_addr = iter_low->first;

    // Find "the range that includes our end point"
    iter_high = range_cache.upper_bound(end_addr);
    iter_high--;
    last_flags = iter_high->second;
    iter_high++;
    // Then find the "range after the one that includes our endpoint"
    if(iter_high != range_cache.end())
        last_addr = iter_high->first;
    else
        last_addr = -1;
    
    // If the range we're inserting touches another range at the start
    // we might have to merge them later.
    if(first_addr == start_addr && iter_low != range_cache.begin()) {
        iter_low--;
        super_previous_addr = iter_low->first;
        super_previous_flags = iter_low->second;
        iter_low++;
    }

    // If the range we're inserting touches another range at the end
    // we might have to merge them later.
    if((end_addr+1) == last_addr && iter_high!= range_cache.end()) {
        super_later_addr = iter_high->first;
        super_later_flags = iter_high->second;
    }

    // Erase everything between our start range and our end range.
    range_cache.erase(iter_low, iter_high);

    // We only insert our new range if we aren't doing the "start merge"
    // If we're on a boundary, and the guy before us has the same flags
    // that guy just covers us.
    if(!(super_previous_addr && super_previous_flags == flag)) {
        // If the above isn't true, then we don't need to do a previous-merge.
        // The question now becomes: Did part of the range we deleted above
        // need to actually stay? If so, add it backin.
        // TODO: modify this to save time.
        if(first_addr != start_addr) {
            if(first_flags != flag)
                range_cache[first_addr] = first_flags;
            else
                start_addr = first_addr;
        }
    }

    // Add our real range in.
    range_cache[start_addr] = flag;

    // If part of the stuff we deleted above is actually the range after
    // what we inserted, add it back in too.
    if((end_addr+1) < last_addr && end_addr != -1 && last_flags != flag)
        range_cache[end_addr+1] = last_flags;
    
    // End merge fixup.
    if(super_later_addr && super_later_flags == flag)
        range_cache.erase(super_later_addr);
}

inline void rm_range(map<unsigned int, unsigned int> &range_cache, unsigned int start_addr, unsigned int end_addr)
{
    add_range(range_cache, start_addr, end_addr, NOT_WATCHED);
}

inline int update_flag(int input_flag, int new_flag, bool add)
{
    if(add)
        return input_flag | new_flag;
    else
        return input_flag & ~(new_flag);
}

void update_range(map<unsigned int, unsigned int> &range_cache, unsigned int start_addr, unsigned int end_addr, int flag, bool add)
{
    map<unsigned int, unsigned int>::iterator iter_low, iter_high, go_iter, temp_iter;

    // Find the "range that includes our start point"
    iter_low = range_cache.upper_bound(start_addr);
    iter_low--;

    if(iter_low->first == start_addr) {
        // If our start address matches this range's start address,
        // we need to see if the range before this one needs merged in.

        // Start by updating the flag in this section.
        temp_iter = iter_low;
        temp_iter++;
        if(temp_iter != range_cache.end() && temp_iter->first > (end_addr+1)) {
            range_cache[end_addr+1] = iter_low->second;
        }
        iter_low->second = update_flag(iter_low->second, flag, add);

        // Check if they're equal now.
        if(iter_low != range_cache.begin()) {
            temp_iter = iter_low;
            temp_iter--;
            if (temp_iter->second == flag) {
                // If they're equal, the latter needn't exist.
                range_cache.erase(iter_low);
            }
            iter_low = temp_iter;
        }
        // OK, we're done with the beginning stuff.
    }
    else {
        // Instead, we need to see if this update will change the current range.
        // If so, we will need to split
        int temp_flag = update_flag(iter_low->second, flag, add);
        if (temp_flag != iter_low->second) {
            // This part is in case our update is at the end of a range..
            temp_iter = iter_low;
            temp_iter++;
            if(temp_iter == range_cache.end() || temp_iter->first > (end_addr+1)) {
                range_cache[end_addr+1] = iter_low->second;
            }
            // Insert our beginning guy.
            range_cache[start_addr] = temp_flag;
            // This will take us into the guy we just inserted
            iter_low++;
            assert(iter_low->first == start_addr);
        }
    }
    // Now our iter_low is the beginning address of the first guy to update
    // after our "start" range.
    iter_low++;

    iter_high = range_cache.upper_bound(end_addr);
    temp_iter = iter_high;
    temp_iter--;

    if(iter_high != range_cache.end()) {
        if((end_addr+1) == iter_high->first) {
            // If our end addr matches up with the following range's end addr,
            // we need to see if they should be merged in.
            temp_iter->second = update_flag(temp_iter->second, flag, add);

            // Check if they're equal now.
            if(temp_iter->second == iter_high->second) {
                // If they're equal, the latter needn't exist.
                range_cache.erase(iter_high);
                iter_high = temp_iter;
            }
            //iter_high++;
        }
        else {
            // We're don't go all the way up to it, so we may need
            // to split.
            int temp_flag = temp_iter->second;
            temp_iter->second = update_flag(temp_iter->second, flag, add);
            if (temp_flag != iter_low->second) {
                range_cache[end_addr] = temp_flag;
                temp_iter++;
            }
            iter_high = temp_iter;
        }
    }
    else {
        // The range directly after us is the end of the list.
        // In other words, it goes until the last address.
        // So we just need to check if this requires a split
        if(end_addr != -1 && (temp_iter)->first != end_addr) {
            // We're not at the end, and this range isn't a 1-byte range
            // So we must split.
            int temp_flag= temp_iter->second;
            temp_iter->second = update_flag(temp_iter->second, flag, add);
            range_cache[end_addr] = temp_flag;
        }
        else {
            // We go all the way to the end, too, so we just need to
            // perform updates down below.
            temp_iter->second = update_flag(temp_iter->second, flag, add);
        }
    }

    if(iter_low->first < iter_high->first) {
        for(go_iter = iter_low; go_iter != iter_high; go_iter++) {
            temp_iter = go_iter;
            temp_iter--;
            go_iter->second = update_flag(go_iter->second, flag, add);
            if(go_iter->second == temp_iter->second) {
                // Merge with the previous guy if our flags match up.
                range_cache.erase(go_iter);
                go_iter = temp_iter;
            }
        }
    }
}

void inline start_thread(int thread_id)
{
    max_thread = thread_id;
}

void read_input_from_file(char *filename)
{
    ifstream ReadFile;

    ReadFile.open(filename);
    if(ReadFile.is_open()) {
        char command[23];
        char start_addr_str[9];
        char end_addr_str[9];
        char thread_id_str[6];
        pin_command command_from_pin;
        while(!ReadFile.getline(command, 23, '\n').eof()) {
            unsigned int start_addr, end_addr;
            int thread_id;
            for(int i = 0; i < 5; i++)
                thread_id_str[i] = command[i+1];
            for(int i = 0; i < 8; i++) {
                start_addr_str[i] = command[i+6];
                end_addr_str[i] = command[i+14];
            }
            thread_id_str[5] = '\0';
            start_addr_str[8] = '\0';
            end_addr_str[8] = '\0';
            sscanf(thread_id_str, "%x", &thread_id);
            sscanf(start_addr_str, "%x", &start_addr);
            sscanf(end_addr_str, "%x", &end_addr);
            if(start_addr > end_addr) {
                assert(0);
            }
            switch(command[0])
            {
                case 'a' :
                    command_from_pin.opcode = 0;
                    break;
                case 'b' :
                    command_from_pin.opcode = 1;
                    break;
                case 'c' :
                    command_from_pin.opcode = 2;
                    break;
                case 'd' :
                    command_from_pin.opcode = 3 ;
                    break;
                case 'e' :
                    command_from_pin.opcode = 4;
                    break;
                case 'f' :
                    command_from_pin.opcode = 5;
                    break;
                case 'g' :
                    command_from_pin.opcode = 6;
                    break;
                case 'h' :
                    command_from_pin.opcode = 7;
                    break;
                case 'i' :
                    start_thread(thread_id);
                    continue;
                case 'j' :
                    command_from_pin.opcode = 8;
                    break;
                case 'k' :
                    command_from_pin.opcode = 9;
                    break;
                case 'z':
                    command_from_pin.opcode = 10;
                    break;
                default:
                    cerr << "Unknown command in the file." << endl;
                    cerr << "Line is: " << command << endl;
                    exit(-1);
            }
            command_from_pin.start_addr = start_addr;
            command_from_pin.end_addr = end_addr;
            command_from_pin.thread_id = thread_id;
            pin_commands.push_back(command_from_pin);
        }
    }
    else {
        cerr << "Could not open file. " << endl;
        exit(-1);
    }
    ReadFile.close();
}

volatile short op;
volatile unsigned short thread;
volatile unsigned int starter;
volatile unsigned int ender;

void walk_input_file()
{
    unsigned int vector_size = pin_commands.size();
    for(unsigned int i = 0; i < vector_size; i++) {
        op = pin_commands[i].opcode;
        thread = pin_commands[i].thread_id;
        starter = pin_commands[i].start_addr;
        ender = pin_commands[i].end_addr;
    }
}

void run_input_file()
{
    unsigned int vector_size = pin_commands.size();
    for(unsigned int i = 0; i < vector_size; i++) {
        short thread_id = pin_commands[i].thread_id;
        unsigned int start_addr = pin_commands[i].start_addr;
        unsigned int end_addr = pin_commands[i].end_addr;
        switch(pin_commands[i].opcode)
        {
            case 0 :
                rm_range(thread_to_rc[thread_id], start_addr, end_addr);
                break;
            case 1 :
                add_range(thread_to_rc[thread_id], start_addr, end_addr, WRITE_WATCHED);
                break;
            case 2 :
                add_range(thread_to_rc[thread_id], start_addr, end_addr, READ_WATCHED);
                break;
            case 3 :
                add_range(thread_to_rc[thread_id], start_addr, end_addr, RW_WATCHED);
                break;
            case 4 :
                update_range(thread_to_rc[thread_id], start_addr, end_addr, READ_WATCHED, true);
                break;
            case 5 :
                update_range(thread_to_rc[thread_id], start_addr, end_addr, WRITE_WATCHED, true);
                break;
            case 6 :
                update_range(thread_to_rc[thread_id], start_addr, end_addr, READ_WATCHED, false);
                break;
            case 7 :
                update_range(thread_to_rc[thread_id], start_addr, end_addr, WRITE_WATCHED, false);
                break;
            case 8 :
                check_range(thread_to_rc[thread_id], start_addr, end_addr, WRITE_WATCHED);
                break;
            case 9:
                check_range(thread_to_rc[thread_id], start_addr, end_addr, READ_WATCHED);
                break;
            case 10:
                print_thing(thread_to_rc[thread_id]);
                break;
            default:
                cerr << "Unknown command in the stack" << pin_commands[i].opcode << endl;
                exit(-1);
        }
    }
}

int main(int argc, char*argv[])
{
    map<unsigned int, unsigned int>::iterator it,iterator_low,iterator_high;
    struct timeval start_time, end_time, difference;
    unsigned long long us_to_walk;
    unsigned long long us_to_work_tree;

    if(argc != 2) {
        cerr << "Please give the file name to parse as a parameter." << endl;
        exit(-1);
    }

    max_thread = 0;

    read_input_from_file(argv[1]);

    // This will properly hold all the threads we need.
    thread_to_rc = new map<unsigned int, unsigned int>[(max_thread+1)];
    // Set the first watchpoint.
    for(int i = 0; i <= max_thread; i++)
        thread_to_rc[i][0] = NOT_WATCHED;

    // Walk it once to warm everything up.
    walk_input_file();

    timerclear(&start_time);
    timerclear(&end_time);
    timerclear(&difference);

    // Walk the data structure once to see how long it takes.
    gettimeofday(&start_time, NULL);
    walk_input_file();
    gettimeofday(&end_time, NULL);
    difference.tv_sec = end_time.tv_sec - start_time.tv_sec;
    difference.tv_usec = end_time.tv_usec - start_time.tv_usec;
    us_to_walk = 1000000*(unsigned long long)difference.tv_sec + (unsigned long long)difference.tv_usec;

    timerclear(&start_time);
    timerclear(&end_time);
    timerclear(&difference);

    // Now time how long it takes to do all the insertions & deletions.
    gettimeofday(&start_time, NULL);
    run_input_file();
    gettimeofday(&end_time, NULL);

    difference.tv_sec = end_time.tv_sec - start_time.tv_sec;
    difference.tv_usec = end_time.tv_usec - start_time.tv_usec;
    us_to_work_tree = 1000000*(unsigned long long)difference.tv_sec + (unsigned long long)difference.tv_usec;

    cout << "This took " << dec << (us_to_work_tree - us_to_walk) << " microseconds." << endl;

}
