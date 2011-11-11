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

//#define FINALIZE_DEBUG
//#define CONSTANT_DEBUG

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
//      10 - Create a bitmap
//      11 - Remove a bitmap
//      12 - Set bits in a bitmap
//      13 - Check bits in a bitmap
//      24 - print this thread
struct pin_command {
    unsigned int start_addr;
    unsigned int end_addr;
    unsigned int thread_id;
    unsigned int opcode;
};

map<unsigned int, unsigned int> thread_to_rc[65536];
map<unsigned int, int*> thread_to_bitmap[65536];

#ifdef FINALIZE_DEBUG
// Bitmap of pointers. We will use these as bitmaps to compare against the ranges.
unsigned char *bitmaps[65536];
vector<int> list_of_threads;
#endif 

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

void verify_thing(map<unsigned int, unsigned int> &range_cache)
{
    map<unsigned int, unsigned int>::iterator range_cache_iter;
    unsigned int previous_address = 0;
    unsigned int previous_flag = 0;
    range_cache_iter = range_cache.begin();
    previous_address = range_cache_iter->first;
    previous_flag = range_cache_iter->second;
    range_cache_iter++;
    while(range_cache_iter != range_cache.end()) {
        if(range_cache_iter->second == previous_flag) {
            fprintf(stdout, "ERROR! Two touching ranges have the same flag!\n");
            fprintf(stdout, "\tFirst addr: 0x%x First flag: 0x%x\n",
                        previous_address, previous_flag);
            fprintf(stdout, "\tSecond addr: 0x%x Second flag: 0x%x\n",
                        range_cache_iter->first, range_cache_iter->second);
            print_thing(range_cache);
            exit(-1);
        }
        previous_address = range_cache_iter->first;
        previous_flag = range_cache_iter->second;
        range_cache_iter++;
    }
}

#ifdef FINALIZE_DEBUG
void validate(int thread_id)
{
    map<unsigned int, unsigned int>::iterator range_cache_iter;
    map<unsigned int, unsigned int>::iterator next_entry;
    unsigned int i;

    printf("Validating thread %d\n", thread_id);

    range_cache_iter = thread_to_rc[thread_id].begin();
    next_entry = range_cache_iter;
    next_entry++;
    while (range_cache_iter != thread_to_rc[thread_id].end()) {
        unsigned int end_addr;
        if (next_entry == thread_to_rc[thread_id].end()) {
            end_addr = -1;
        }
        else
            end_addr = next_entry->first - 1;;

        for (i = range_cache_iter->first; i <= end_addr; i++) {
            unsigned char bitmap_flag = bitmaps[thread_id][i>>2];
            bool correct = false;
            unsigned char bad_flag = 0;
            if (i%4 == 0) {
                if ((bitmap_flag & 0x3) == range_cache_iter->second)
                    correct = true;
                else
                    bad_flag = bitmap_flag & 0x3;
            }
            else if (i%4 == 1) {
                if (((bitmap_flag & 0xc)>>2) == range_cache_iter->second)
                    correct = true;
                else
                    bad_flag = (bitmap_flag & 0xc) >> 2;
            }
            else if (i%4 == 2) {
                if (((bitmap_flag & 0x30)>>4) == range_cache_iter->second)
                    correct = true;
                else
                    bad_flag = (bitmap_flag & 0x30) >> 4;
            }
            else if (i%4 == 3) {
                if (((bitmap_flag & 0xc0)>>6) == range_cache_iter->second)
                    correct = true;
                else
                    bad_flag = (bitmap_flag & 0xc0) >> 6;
            }

            if (!correct) {
                fprintf(stderr, "ERROR VALIDATING THREAD %d\n", thread_id);
                fprintf(stderr, "Range cache entry says 0x%x-0x%x : %x\n", range_cache_iter->first, end_addr, range_cache_iter->second);
                fprintf(stderr, "Bitmap entry 0x%x says: %x\n", i, bad_flag);
                fprintf(stderr, "Total bitmap is %x\n", bitmap_flag);
                exit(-1);
            }
            if(i == 0xffffffff)
                break;
        }
        range_cache_iter = next_entry;
        next_entry++;
    }
}
#endif

bool check_range(map<unsigned int, unsigned int> &range_cache, unsigned int start_addr, unsigned int end_addr, unsigned int flag)
{
    map<unsigned int, unsigned int>::iterator iter;
    iter = range_cache.upper_bound(start_addr);
    iter--;
    while(iter != range_cache.end() && iter->first <= end_addr) {
        if(iter->second | flag)
            return true;
        else
            iter++;
    }
    return false;
}

void add_range(map<unsigned int, unsigned int> &range_cache, unsigned int start_addr, unsigned int end_addr, unsigned int flag)
{
    map<unsigned int, unsigned int>::iterator iter_low, iter_high, iter_low_temp, iter_high_temp;
    unsigned int first_addr = 0;
    unsigned int last_addr = -1;
    int first_flags, last_flags;
    int super_previous_flags, super_later_flags;
    bool super_prev_exists = false;
    bool super_later_exists = false;

    // Find the "range that includes our start point"
    iter_low = range_cache.upper_bound(start_addr);
    iter_high_temp = iter_low;
    iter_low--;
    first_flags = iter_low->second;
    first_addr = iter_low->first;

    // If the range we're inserting touches another range at the start
    // we might have to merge them later.
    if(first_addr == start_addr && iter_low != range_cache.begin()) {
        iter_low_temp = iter_low;
        iter_low_temp--;
        super_previous_flags = iter_low_temp->second;
        super_prev_exists = true;
    }

    // Find "the range that includes our end point". This ends up in iter_high.
    // iter_high follows iter_high_temp by one node each time.
    last_flags = first_flags;
    while(iter_high_temp != range_cache.end() && iter_high_temp->first <= end_addr)
    {
        iter_high = iter_high_temp;
        last_flags = iter_high->second;
        iter_high_temp++;
        range_cache.erase(iter_high);
    }

    // The "range after the one that includes our endpoint" will now move into
    // iter_high.
    iter_high = iter_high_temp;
    if(iter_high != range_cache.end())
        last_addr = iter_high->first;
    else // If we reach the end, then the last addr is the last available.
        last_addr = -1;
    
    // If the range we're inserting touches another range at the end
    // we might have to merge them later.
    if((end_addr+1) == last_addr && iter_high!= range_cache.end())
        super_later_exists = true;

    // If the range you're trying to insert touches up a previous range..
    if(super_prev_exists) {
        // And it has the same flags you're trying to insert,
        // Then remove the existing range at that area, because the previous
        // range now extends inwards until the end of this new range.
        if(super_previous_flags == flag)
            range_cache.erase(iter_low);
        else {
            // Otherwise, the old range starting at this address continues to
            // exist, except with a new flag.
            if(first_flags != flag) // only if the flag changes!
                range_cache[start_addr] = flag;
        }
    }
    else {
        // If not up against anyone, then we need to insert the new range.
        if(first_flags != flag) // only if the flag changes!
            range_cache[start_addr] = flag;
    }

    // If part of the stuff we deleted above is actually the range after
    // what we inserted, add it back in too.
    if((end_addr+1) < last_addr && end_addr != -1 && last_flags != flag)
        range_cache[end_addr+1] = last_flags;
    
    // End merge fixup.
    if(super_later_exists && iter_high->second == flag)
        range_cache.erase(iter_high);
}

inline void rm_range(map<unsigned int, unsigned int> &range_cache, unsigned int start_addr, unsigned int end_addr)
{
    add_range(range_cache, start_addr, end_addr, NOT_WATCHED);
}

inline unsigned int update_flag(unsigned int input_flag, unsigned int new_flag, bool add)
{
    if(add)
        return input_flag | new_flag;
    else
        return input_flag & ~(new_flag);
}

void update_range(map<unsigned int, unsigned int> &range_cache, unsigned int start_addr, unsigned int end_addr, unsigned int flag, bool add)
{
    map<unsigned int, unsigned int>::iterator iter_low, iter_high, go_iter, forward_iter, temp_iter, super_low_iter;
	unsigned int temp_flag;

    // Find the "range that includes our start point"
    iter_low = range_cache.upper_bound(start_addr);
    temp_iter = iter_low;
    iter_low--;

	// We need to see what the starting range's new flag will be.
	temp_flag = update_flag(iter_low->second, flag, add);

    // If our start address matches this range's start address,
    // we need to see if the range before this one needs merged in.
	if(iter_low->first == start_addr) {
		if(temp_flag != iter_low->second) {
			// Start by making sure that if our update-end doesn't reach
			// the end of this range, we split.
            if (temp_iter != range_cache.end()) {
                if (temp_iter->first > (end_addr+1)) {
                    range_cache[end_addr+1] = iter_low->second;
                }
            }
            else if (end_addr != -1) {
                range_cache[end_addr+1] = iter_low->second;
            }
			if (iter_low != range_cache.begin()) {
				// If you're not first in the list, check for merge condition.
				super_low_iter = iter_low;
				super_low_iter--;
				if (super_low_iter->second == temp_flag) {
					// If they're equal, the latter needn't exist.
					range_cache.erase(iter_low);
					iter_low = super_low_iter;
				}
				else {
					// If they're not equal, then we should just update this range.
					iter_low->second = temp_flag;
				}
			}
			else // If you're the first guy, just update your flag.
				iter_low->second = temp_flag;
		}
	}
	else {
        // Instead, we need to see if this update will change the current range.
        // If so, we will need to split
        if (temp_flag != iter_low->second) {
			// Start by making sure that if our update-end doesn't reach
			// the end of this range, we split.
            if(temp_iter != range_cache.end()) {
				if (temp_iter->first > (end_addr+1)) {
					range_cache[end_addr+1] = iter_low->second;
				}
			}
			else if (end_addr != -1) {
				range_cache[end_addr+1] = iter_low->second;
			}
            // Insert the new flag into the newly-formed range.
            range_cache[start_addr] = temp_flag;
            // This will take us into the guy we just inserted
            iter_low++;
        }
    }
	// OK, we're done with the beginning range.
    
	// Now our iter_low is the beginning address of the first guy to update
    // after our "start" range.
	// temp_iter is a follower.
	temp_iter = iter_low;
    iter_low++;

    if (iter_low->second == temp_iter->second) {
        range_cache.erase(iter_low);
        iter_low = temp_iter;
        iter_low++;
    }

	// As long as we haven't gone past end_addr, we still need to update stuff.
	if (iter_low != range_cache.end() && (iter_low->first <= end_addr)) {
		// This will go through ranges, merging and updating flags as it goes.
		go_iter = forward_iter = iter_low;
		while (go_iter != range_cache.end() && go_iter->first <= end_addr) {
			// Lookahead iterator to see if we've reached update-end.
			forward_iter++;
			temp_flag = update_flag(go_iter->second, flag, add);

			// Only need to do most work if we changed the flags at all.
			if (temp_flag != go_iter->second) {
				// Start by making sure that if our update-end doesn't reach
				// the end of this range, we split.
				if (forward_iter != range_cache.end()) {
					if (forward_iter->first > (end_addr+1))
						range_cache[end_addr+1] = go_iter->second;
				}
				else if(end_addr != -1) {
					range_cache[(end_addr+1)] = go_iter->second;
				}

				// If this range's new flag matches the previous range's..
				if(temp_flag == temp_iter->second) {
					// Merge with the previous range.
					range_cache.erase(go_iter);
					go_iter = temp_iter;
				}
				else // Otherwise just update this range.
					go_iter->second = temp_flag;
			}
			temp_iter = go_iter;
			go_iter = forward_iter;
		}
	}
    else {
        go_iter = iter_low;
    }

	// temp_iter now contains the last range we updated.
	// go_iter either contains rc.end() or one after the range we wanted to update.
    iter_high = go_iter;

    if(iter_high != range_cache.end()) {
        if((end_addr+1) == iter_high->first) {
            // If our end addr matches up with the following range's end addr,
            // we need to see if they should be merged together.
            if(temp_iter->second == iter_high->second) {
                // If they're equal, the latter needn't exist.
                range_cache.erase(iter_high);
            }
        }
    }
}

void create_bitmap(int thread_id, unsigned int start_addr)
{
    // Each bitmap covers a page of memory.
    unsigned int page_num = start_addr >> 12;
    // Two WP bits per real byte means that it takes 1K of memory to hold
    // watchpoints for 4K of data.
    (thread_to_bitmap[thread_id])[page_num] = (int*) malloc(1024);
}

void erase_bitmap(int thread_id, unsigned int start_addr)
{
    // Each bitmap covers a page of memory.
    unsigned int page_num = start_addr >> 12;
    free((thread_to_bitmap[thread_id])[page_num]);
}

void set_bitmap_bits(int thread_id, unsigned int start_addr, unsigned int end_addr)
{
    // Each bitmap covers a page of memory.
    unsigned int page_num = start_addr >> 12;
    int* bitmap_ptr;
    unsigned int start_int, end_int;

    bitmap_ptr = (thread_to_bitmap[thread_id])[page_num];

    start_int = (start_addr & 0xfff) >> 4;
    end_int = (end_addr & 0xfff) >> 4;
    for(;start_int <= end_int; start_int++)
    {
        bitmap_ptr[start_int] = 0xff;
    }
}

bool check_bitmap_bits(int thread_id, unsigned int start_addr, unsigned int end_addr)
{
    unsigned int page_num = start_addr >> 12;
    int* bitmap_ptr;
    unsigned int start_int, end_int;

    bitmap_ptr = (thread_to_bitmap[thread_id])[page_num];

    start_int = (start_addr & 0xfff) >> 4;
    end_int = (end_addr & 0xfff) >> 4;
    for(; start_int <= end_int; start_int++)
    {
        if(bitmap_ptr[start_int] == 0xfe)
            return true;
    }
    return false;
}

void inline start_thread(int thread_id)
{
    thread_to_rc[thread_id][0] = NOT_WATCHED;
#ifdef FINALIZE_DEBUG
    list_of_threads.push_back(thread_id);
    bitmaps[thread_id] = (unsigned char*)malloc(1073741824);
    if(bitmaps[thread_id] == NULL) {
        fprintf(stderr, "Couldn't allocate debug bitmap for thread %d\n", thread_id);
        exit(-1);
    }
#endif 
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
                case 'l' :
                    command_from_pin.opcode = 10;
                    break;
                case 'm' :
                    command_from_pin.opcode = 11;
                    break;
                case 'n' :
                    command_from_pin.opcode = 12;
                    break;
                case 'o' :
                    command_from_pin.opcode = 13;
                    break;
                case 'z':
                    command_from_pin.opcode = 24;
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

volatile unsigned int op;
volatile unsigned int thread;
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
#ifdef FINALIZE_DEBUG
    unsigned int debug_x;
    unsigned char debug_flag;
    unsigned char debug_old_flag;
#endif
    unsigned int vector_size = pin_commands.size();
    for(unsigned int i = 0; i < vector_size; i++) {
        unsigned int thread_id = pin_commands[i].thread_id;
        unsigned int start_addr = pin_commands[i].start_addr;
        unsigned int end_addr = pin_commands[i].end_addr;
        switch(pin_commands[i].opcode)
        {
            case 0 :
                rm_range(thread_to_rc[thread_id], start_addr, end_addr);
#ifdef FINALIZE_DEBUG
                for(debug_x = start_addr; debug_x <= end_addr; debug_x++) {
                    debug_flag = (3 << ((debug_x%4)*2));
                    debug_flag = ~debug_flag;
                    bitmaps[thread_id][debug_x>>2] &= debug_flag;
                    if(debug_x == 0xffffffff)
                        break;
                }
#endif
                break;
            case 1 :
                add_range(thread_to_rc[thread_id], start_addr, end_addr, WRITE_WATCHED);
#ifdef FINALIZE_DEBUG
                for(debug_x = start_addr; debug_x <= end_addr; debug_x++) {
                    debug_flag = (3 << ((debug_x%4)*2));
                    debug_flag = ~debug_flag;
                    bitmaps[thread_id][debug_x>>2] &= debug_flag;
                    debug_flag = (WRITE_WATCHED << ((debug_x%4)*2));
                    bitmaps[thread_id][debug_x>>2] |= debug_flag;
                    if(debug_x == 0xffffffff)
                        break;
                }
#endif
                break;
            case 2 :
                add_range(thread_to_rc[thread_id], start_addr, end_addr, READ_WATCHED);
#ifdef FINALIZE_DEBUG
                for(debug_x = start_addr; debug_x <= end_addr; debug_x++) {
                    debug_flag = (3 << ((debug_x%4)*2));
                    debug_flag = ~debug_flag;
                    bitmaps[thread_id][debug_x>>2] &= debug_flag;
                    debug_flag = (READ_WATCHED << ((debug_x%4)*2));
                    bitmaps[thread_id][debug_x>>2] |= debug_flag;
                    if(debug_x == 0xffffffff)
                        break;
                }
#endif
                break;
            case 3 :
                add_range(thread_to_rc[thread_id], start_addr, end_addr, RW_WATCHED);
#ifdef FINALIZE_DEBUG
                for(debug_x = start_addr; debug_x <= end_addr; debug_x++) {
                    debug_flag = (3 << ((debug_x%4)*2));
                    debug_flag = ~debug_flag;
                    bitmaps[thread_id][debug_x>>2] &= debug_flag;
                    debug_flag = (RW_WATCHED << ((debug_x%4)*2));
                    bitmaps[thread_id][debug_x>>2] |= debug_flag;
                    if(debug_x == 0xffffffff)
                        break;
                }
#endif
                break;
            case 4 :
                update_range(thread_to_rc[thread_id], start_addr, end_addr, READ_WATCHED, true);
#ifdef FINALIZE_DEBUG
                for(debug_x = start_addr; debug_x <= end_addr; debug_x++) {
                    debug_flag = (READ_WATCHED << ((debug_x%4)*2));
                    bitmaps[thread_id][debug_x>>2] |= debug_flag;
                    if(debug_x == 0xffffffff)
                        break;
                }
#endif
                break;
            case 5 :
                update_range(thread_to_rc[thread_id], start_addr, end_addr, WRITE_WATCHED, true);
#ifdef FINALIZE_DEBUG
                for(debug_x = start_addr; debug_x <= end_addr; debug_x++) {
                    debug_flag = (WRITE_WATCHED << ((debug_x%4)*2));
                    bitmaps[thread_id][debug_x>>2] |= debug_flag;
                    if(debug_x == 0xffffffff)
                        break;
                }
#endif
                break;
            case 6 :
                update_range(thread_to_rc[thread_id], start_addr, end_addr, READ_WATCHED, false);
#ifdef FINALIZE_DEBUG
                for(debug_x = start_addr; debug_x <= end_addr; debug_x++) {
                    debug_flag = ~(READ_WATCHED << ((debug_x%4)*2));
                    bitmaps[thread_id][debug_x>>2] &= debug_flag;
                    if(debug_x == 0x847ace0)
                        fprintf(stderr,  "update flag is 0x%x\n", debug_flag);
                    if(debug_x == 0xffffffff)
                        break;
                }
#endif
                break;
            case 7 :
                update_range(thread_to_rc[thread_id], start_addr, end_addr, WRITE_WATCHED, false);
#ifdef FINALIZE_DEBUG
                for(debug_x = start_addr; debug_x <= end_addr; debug_x++) {
                    debug_flag = ~(WRITE_WATCHED << ((debug_x%4)*2));
                    bitmaps[thread_id][debug_x>>2] &= debug_flag;
                    if(debug_x == 0xffffffff)
                        break;
                }
#endif
                break;
            case 8 :
                check_range(thread_to_rc[thread_id], start_addr, end_addr, WRITE_WATCHED);
                break;
            case 9:
                check_range(thread_to_rc[thread_id], start_addr, end_addr, READ_WATCHED);
                break;
            case 10:
                create_bitmap(thread_id, start_addr);
                break;
            case 11:
                erase_bitmap(thread_id, start_addr);
                break;
            case 12:
                set_bitmap_bits(thread_id, start_addr, end_addr);
                break;
            case 13:
                check_bitmap_bits(thread_id, start_addr, end_addr);
                break;
            case 24:
                print_thing(thread_to_rc[thread_id]);
                break;
            default:
                cerr << "Unknown command in the stack" << pin_commands[i].opcode << endl;
                exit(-1);
        }
#ifdef CONSTANT_DEBUG
        verify_thing(thread_to_rc[thread_id]);
#endif
    }
}

int main(int argc, char*argv[])
{
    map<unsigned int, unsigned int>::iterator it,iterator_low,iterator_high;
    struct timeval start_time, end_time, difference;
    unsigned int long us_to_walk;
    unsigned int long us_to_work_tree;

    if(argc != 2) {
        cerr << "Please give the file name to parse as a parameter." << endl;
        exit(-1);
    }
    read_input_from_file(argv[1]);

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
    us_to_walk = 1000000*(unsigned int long)difference.tv_sec + (unsigned int long)difference.tv_usec;

    timerclear(&start_time);
    timerclear(&end_time);
    timerclear(&difference);

    // Now time how long it takes to do all the insertions & deletions.
    gettimeofday(&start_time, NULL);
    run_input_file();
    gettimeofday(&end_time, NULL);

#ifdef FINALIZE_DEBUG
    for(vector<int>::iterator list_iter = list_of_threads.begin(); list_iter != list_of_threads.end(); list_iter++) {
        validate(*list_iter);
        verify_thing(*list_iter);
    }
#endif

    difference.tv_sec = end_time.tv_sec - start_time.tv_sec;
    difference.tv_usec = end_time.tv_usec - start_time.tv_usec;
    us_to_work_tree = 1000000*(unsigned int long)difference.tv_sec + (unsigned int long)difference.tv_usec;

    cout << "This took " << dec << (us_to_work_tree - us_to_walk) << " microseconds." << endl;
}
