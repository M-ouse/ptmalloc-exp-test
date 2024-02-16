### Tested on ptmalloc-2.35, Linux 6.5.0-17-generic #17~22.04.1-Ubuntu 

This is a simple trick to make ptmalloc's free check futile.

We all know ptmalloc added tcache for efficiency, but the cons are lacking some security checks than non-tcache free procedure.

When the chunk to be freed has the size within tcache's domain, and corresponding tcache entry has room with redundancy,
ptmalloc will put the chunk into tcache without many checks, which actually reduces security integrity.

In this case, we first do 3 mallocs: p1 = malloc(sz) p2 = malloc(sz) p3 = malloc(sz). (sz < tcache.size_threshold)
If we can use an overflow from chunk p1 to wirte p2.size with sz2 (sz < sz2 <= tcache.size_threshold), then free(p2), chunk p2
will not receive strict check, but be put into tcache directly. Free p1, p3, we got tcache_sz [p3 -> p1], tcache_sz2 [ p2 ].
Then we do p4 = malloc(sz3) (usable_size(sz3) = sz2), we will get p4 == p2, p4 and p3 has an overlapping. Next we can modify
p3->next to arbitrary address (tcache poisoning), which is taken as an example, of course we can do further attack since we already
have a chunk overlapping.
