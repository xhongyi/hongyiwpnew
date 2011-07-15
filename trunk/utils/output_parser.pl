#!/usr/bin/perl
#This script goes through the output from our Unlimited Watchpoint pintool
#and gathers up all the data from each thread's independent output.
#It adds them all together and places them so we can more easily add them
#to our spreadsheets.

my $instruction_count;
my $total_memory_ops;
my $shared_mem_ops;
my $watchpoint_sets_and_removes;
my $vm_faults;
my $vm_bitmap_changes;
my $multi_thread_vm_faults;
my $multi_thread_vm_bitmap_changes;
my $memtracker_tl1_misses;
my $memtracker_write_misses;
my $memtracker_bitmap_writes;
my $mmp_sst_wlb_misses;
my $mmp_sst_ranges_moved;
my $mmp_mlpt_wlb_misses;
my $mmp_mlpt_mem_reads;
my $mmp_mlpt_mem_writes;
my $range_misses;
my $range_kickouts;
my $range_ocbm_misses;
my $range_ocbm_kickouts;

if(scalar @ARGV != 1) {
    print "Incorrect calling method.\n";
    print "Shold be $0 {input file}\n";
    print "type \"$0 -h\" to see a bit of help\n";
}

if($ARGV[0] eq "-h") {
    print "This will take the outputs from the Unlimted Watchpoint pintool\n";
    print "and add all of the individual therads' stats together.\n";
    print "To run, type \"$0 {input file}\"\n";
}
if(! open READFILE, "<", $ARGV[0]) {
    die "couldn't open input file $ARGV[0]\n";
}

my @split_fields;

while(<READFILE>)
{
    chomp;
    @split_fields = split;
    if(m/number of instructions/) {
        $instruction_count += $split_fields[-1];
    }
    elsif(m/Checks for watchpoint faults/) {
        $total_memory_ops += $split_fields[-1];
    }
    elsif(m/Watchpoint faults taken/) {
        $shared_mem_ops += $split_fields[-1];
    }
    elsif(m/Watchpoint 'set/ || m/Watchpoint 'update/) {
        $watchpoint_sets_and_removes += $split_fields[-1];
    }
    elsif(m/Page table \(multi\) faults/) {
        $vm_faults += $split_fields[-1];
    }
    elsif(m/Page table \(multi\) bitmap/) {
        $vm_bitmap_changes += $split_fields[-1];
    }
    elsif(m/Page table \(single\) faults/) {
        $multi_thread_vm_faults += $split_fields[-1];
    }
    elsif(m/Page table \(single\) bitmap/) {
        $multi_thread_vm_bitmap_changes += $split_fields[-1];
    }
    elsif(m/MemTracker read/) {
        $memtracker_tl1_misses += $split_fields[-1];
    }
    elsif(m/MemTracker write/) {
        $memtracker_write_misses += $split_fields[-1];
    }
    elsif(m/total wp write size/) {
        $memtracker_bitmap_writes += $split_fields[-1];
    }
    elsif(m/Range Cache \(single\) read miss/ || m/Range Cache \(single\) write miss/) {
        $range_misses += $split_fields[-1];
    }
    elsif(m/Range Cache \(single\) kickouts dirty/) {
        $range_kickouts += $split_fields[-1];
    }
    elsif(m/Range Cache \(OCBM\) read miss/ || m/Range Cache \(OCBM\) write miss/) {
        $range_ocbm_misses += $split_fields[-1];
    }
    elsif(m/Range Cache \(OCBM\) kickouts dirty/) {
        $range_ocbm_kickouts += $split_fields[-1];
    }
}

print "Number of instructions: " . $instruction_count . "\n";
print "Total memory ops: " . $total_memory_ops . "\n";
print "Shared memory ops: " . $shared_mem_ops . "\n";
print "Number of sets/removes: " . $watchpoint_sets_and_removes . "\n";
print "VM False Faults: " . ($vm_faults-$shared_mem_ops) . "\n";
print "VM Bitmap changes: " . $vm_bitmap_changes . "\n";
print "Per-Thread VM Faults: " . ($multi_thread_vm_faults-$shared_mem_ops) . "\n";
print "Per-Thread VM Bitmap changes: " . $multi_thread_vm_bitmap_changes . "\n";
print "MemTracker TL1 Read Misses: " . $memtracker_tl1_misses . "\n";
print "MemTracker TL1 Write Misses: " . $memtracker_write_misses . "\n";
print "MemTracker Total TL1 Misses: " . ($memtracker_tl1_misses + $memtracker_write_misses) . "\n";
print "MemTracker Bitmap Writes: " . $memtracker_bitmap_writes . "\n";
print "Range Cache misses: " . $range_misses . "\n";
print "Range Cache kickouts: " . $range_kickouts . "\n";
print "Range Cache + OCBM misses: " . $range_ocbm_misses . "\n";
print "Range Cache + OCBM kickouts: " . $range_ocbm_kickouts . "\n";
