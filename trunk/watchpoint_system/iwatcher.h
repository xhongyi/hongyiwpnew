/*
pseudocode for iwatcher
general fault:
   if (hit in l1) : update lru in l1, and also in l2 if found
   if (hit in l2) : push into l1, update lru
   if (hit in victim cache) : push into l1 and l2, remove in victim cache
   if (miss) : check for availability in VM, if available => bring in required cache line (to act like a real cache)
                                             if unavailable => fetch only watched cache lines, then mark as avaiable in VM.
   if (l2 kickout) : push it in victim cache, mark the page as unavailable if necessary (i.e. check it before set it)
wp_operation:
   if (hit) : update lru
   add_watchpoint : mark the page as unavailable
   rm_watchpoint : if (originally available) --- do nothing
                   else check if the page should be available now

conclusion:
1. only write in l1 and l2 when read miss (no write allocate)
2. only kickouts from l2 are written in victim cache
3. l2 kickouts may cause VM availability to change (l1 and victim cache kickouts doesn't matter)
4. wp_operation only modifies VM availability and lru
*/
