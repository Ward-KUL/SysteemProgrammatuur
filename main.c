/**
 * \author Ward Smets
 */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "ma_malloc.h"

//You need to make a sketch of your memory pool at different points in your program indicated by a number
//Which parts are free, which are allocated?
//Where are your headers/footers - how many bytes do they take? What is stored in it?
//Where are the pointers returned by ma_malloc?

int main(int argc, char *argv[]) {
    char *ptr[] = {NULL, NULL, NULL};
//sketch 1
    ma_init();
//sketch 2
    assert(NULL == ma_malloc(600)); //->should return NULL
    assert(NULL != ma_malloc(200)); //->should not return NULL;

    ma_init();
    ptr[0] = ma_malloc(400); //->should not return NULL (ma_init() clears all)
//sketch 3
    assert(ptr[0] != NULL);
    // for debugging:
    // memset(ptr[0], 'a', 400); //-> makes this area easy to find when using GDB
    ma_free(ptr[0]);

    ptr[0] = ma_malloc(200); //-> should not return NULL
    assert(ptr[0] != NULL);
    // for debugging:
    // memset(ptr[0], 'b', 200); //-> makes this area easy to find when using GDB

    ptr[1] = ma_malloc(100); //-> should not return NULL
    assert(ptr[1] != NULL);
    // for debugging:
    // memset(ptr[1], 'b', 100); //-> makes this area easy to find when using GDB or memdump-tool

    ptr[2] = ma_malloc(100); //-> should not return NULL
//sketch 4
    assert(ptr[2] != NULL);
    // for debugging:
    // memset(ptr[2], 'c', 100); //-> makes this area easy to find when using GDB or memdump-tool

    ma_free(ptr[1]);
//sketch 5
    ma_free(ptr[2]); //-> mind the sequence, first free 1 than 2
//sketch 6
    assert(NULL != ma_malloc(250)); //-> should not return NULL (test for coalescing)

    return 0;
}
