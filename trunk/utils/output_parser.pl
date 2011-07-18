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
my $memtracker_bitmap_reads;
my $memtracker_bitmap_writes;
my $mmp_sst_ranges_moved;
my $mmp_mlpt_wlb_misses;
my $mmp_mlpt_mem_writes;
my $mmp_mlpt_mem_reads;
my $range_misses;
my $range_kickouts;
my $range_complex;
my $range_ocbm_misses;
my $range_ocbm_kickouts;
my $range_ocbm_complex;
my $range_offcbm_misses;
my $range_offcbm_kickouts;
my $range_offcbm_wlb_misses;
my $range_offcbm_to_bitmap;
my $range_offcbm_from_bitmap;
my $range_offcbm_words_changed;
my $range_offcbm_complex;

my $have_seen_vm;
my $have_seen_mt_vm;
my $have_seen_memtracker;
my $have_seen_mmp;
my $have_seen_rc;
my $have_seen_rc_ocbm;
my $have_seen_rc_offcbm;

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
        $have_seen_vm = 1;
    }
    elsif(m/Page table \(multi\) bitmap/) {
        $vm_bitmap_changes += $split_fields[-1];
        $have_seen_vm = 1;
    }
    elsif(m/Page table \(single\) faults/) {
        $multi_thread_vm_faults += $split_fields[-1];
        $have_seen_mt_vm = 1;
    }
    elsif(m/Page table \(single\) bitmap/) {
        $multi_thread_vm_bitmap_changes += $split_fields[-1];
        $have_seen_mt_vm = 1;
    }
    elsif(m/MemTracker read/) {
        $memtracker_tl1_misses += $split_fields[-1];
        $have_seen_memtracker = 1;
    }
    elsif(m/MemTracker write/) {
        $memtracker_write_misses += $split_fields[-1];
    }
    elsif(m/MemTracker total bitmap/) {
        $memtracker_bitmap_reads += $split_fields[-1];
    }
    elsif(m/MemTracker 32/) {
        $memtracker_bitmap_writes += $split_fields[-1];
    }
    elsif(m/Range Cache \(single\) read miss/ || m/Range Cache \(single\) write miss/) {
        $range_misses += $split_fields[-1];
        $have_seen_rc = 1;
    }
    elsif(m/Range Cache \(single\) kickouts dirty/) {
        $range_kickouts += $split_fields[-1];
    }
    elsif(m/ Range Cache \(single\) complex/) {
        $range_complex += $split_fields[-1];
    }
    elsif(m/Range Cache \(OCBM\) read miss/ || m/Range Cache \(OCBM\) write miss/) {
        $range_ocbm_misses += $split_fields[-1];
        $have_seen_rc_ocbm = 1;
    }
    elsif(m/Range Cache \(OCBM\) kickouts dirty/) {
        $range_ocbm_kickouts += $split_fields[-1];
    }
    elsif(m/Range Cache \(OCBM\) complex/) {
        $range_ocbm_complex += $split_fields[-1];
    }
    elsif(m/Range Cache \(OffCBM\) read miss/ || m/Range Cache\(OffCBM\) write miss/) {
        $have_seen_rc_offcbm=1;
        $range_offcbm_misses += $split_fields[-1];
    }
    elsif(m/Range Cache \(OffCBM\) kickouts dirty/) {
        $range_offcbm_kickouts += $split_fields[-1];
    }
    elsif(m/Range Cache \(OffCBM\) wlb miss/) {
        $range_offcbm_wlb_misses += $split_fields[-1];
    }
    elsif(m/Range Cache \(OffCBM\) switches to offcbm/) {
        $range_offcbm_to_bitmap += $split_fields[-1];
    }
    elsif(m/Range Cache \(OffCBM\) switches to ranges/) {
        $range_offcbm_from_bitmap += $split_fields[-1];
    }
    elsif(m/Range Cache \(OffCBM\) 32bit/) {
        $range_offcbm_words_changed += $split_fields[-1];
    }
    elsif(m/Range Cache \(OffCBM\) complex/) {
        $range_offcbm_complex += $split_fields[-1];
    }
    elsif(m/complexity to do SST insertions/) {
        $mmp_sst_ranges_moved += $split_fields[-1];
    }
    elsif(m/2 level PT trie \(single\) plb/) {
        $have_seen_mmp=1;
        $mmp_mlpt_wlb_misses += $split_fields[-1];
    }
    elsif(m/2 level PT trie \(single\) bit/) {
        $mmp_mlpt_mem_writes += $split_fields[-1];
    }
    elsif(m/2 level PT trie \(single\) data loaded/) {
        $mmp_mlpt_mem_reads += $split_fields[-1];
    }
}

print "Number of instructions: " . $instruction_count . "\n";
print "Total memory ops: " . $total_memory_ops . "\n";
print "Shared memory ops: " . $shared_mem_ops . "\n";
print "Number of sets/removes: " . $watchpoint_sets_and_removes . "\n";
if($have_seen_vm==1) {
    print "VM False Faults: " . ($vm_faults-$shared_mem_ops) . "\n";
    print "VM Bitmap changes: " . $vm_bitmap_changes . "\n";
}
if($have_seen_mt_vm==1) {
    print "Per-Thread VM Faults: " . ($multi_thread_vm_faults-$shared_mem_ops) . "\n";
    print "Per-Thread VM Bitmap changes: " . $multi_thread_vm_bitmap_changes . "\n";
}
if($have_seen_memtracker==1) {
    print "MemTracker TL1 Read Misses: " . $memtracker_tl1_misses . "\n";
    print "MemTracker TL1 Write Misses: " . $memtracker_write_misses . "\n";
    print "MemTracker Total TL1 Misses: " . ($memtracker_tl1_misses + $memtracker_write_misses) . "\n";
    print "MemTracker Bitmap Reads: " . $memtracker_bitmap_reads . "\n";
    print "MemTracker Bitmap Writes: " . $memtracker_bitmap_writes . "\n";
}
if($have_seen_mmp==1) {
    print "MMP-SST Ranges Moved: " . $mmp_sst_ranges_moved . "\n";
    print "MMP WLB Misses: " . $mmp_mlpt_wlb_misses . "\n";
    print "MMP MemReads: " . $mmp_mlpt_mem_reads . "\n";
    print "MMP MemWrites: " . $mmp_mlpt_mem_writes . "\n";
}
if($have_seen_rc==1) {
    print "Range Cache misses: " . $range_misses . "\n";
    print "Range Cache kickouts: " . $range_kickouts . "\n";
    print "Range Cache complex updates: " . $range_complex . "\n";
}
if($have_seen_rc_ocbm==1) {
    print "Range Cache + OCBM misses: " . $range_ocbm_misses . "\n";
    print "Range Cache + OCBM kickouts: " . $range_ocbm_kickouts . "\n";
    print "Range Cache + OCBM complex updates: " . $range_ocbm_complex . "\n";
}
if($have_seen_rc_offcbm==1) {
    print "Range Cache + OffCBM misses: " . $range_offcbm_misses . "\n";
    print "Range Cache + OffCBM kickouts: " . $range_offcbm_kickouts . "\n";
    print "Range Cache + OffCBM WLB misses: " . $range_offcbm_wlb_misses . "\n";
    print "Range Cache + OffCBM Switch To Bitmap: " . $range_offcbm_to_bitmap . "\n";
    print "Range Cache + OffCBM Switch To Range: " . $range_offcbm_from_bitmap . "\n";
    print "Range Cache + OffCBM Words Changed: " . $range_offcbm_words_changed . "\n";
    print "Range Cache + OffCBM complex updates: " . $range_offcbm_complex . "\n";
}
