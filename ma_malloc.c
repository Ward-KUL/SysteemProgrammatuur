/**
 * \author Ward Smets
 */

#include "ma_malloc.h"
#include <stddef.h>

#define MEM_POOL_SIZE 600   //in bytes
typedef unsigned char byte;

//! An enum of statuses that can be used in the header
typedef enum {
    ALLOCATED, FREE
} mem_status;

//! Every item in the pool will have at least a header with size and status of the following data
typedef struct {
    size size;
    mem_status status;
} mem_chunk_header;

typedef struct{
    size size;
    mem_status status;
}
mem_chunk_footer;


static byte mem_pool[MEM_POOL_SIZE];

/**
 * Allocates array of bytes (memory pool) and initializes the memory allocator.
 * If some bytes have been used after calling ma_malloc(size), calling to ma_init() will result in clearing up the memory pool.
 */
void ma_init() {

    //TODO: add your code here
    for(int i = 0; i<MEM_POOL_SIZE;i ++){
        mem_pool[i] = 0;
    }
    mem_chunk_header* mainHeader =  (mem_chunk_header*)&mem_pool;
    mainHeader->size = MEM_POOL_SIZE - sizeof(mem_chunk_header) - sizeof(mem_chunk_footer);
    mainHeader->status = FREE;
    mem_chunk_footer* mainFooter = (mem_chunk_footer*)&mem_pool + sizeof(mem_chunk_header);
    mainFooter->status = FREE;
    mainFooter->size = MEM_POOL_SIZE - sizeof(mem_chunk_header) - sizeof(mem_chunk_footer);

}

/**
 * Requesting for the tsize bytes from memory pool.
 * If the request is possible, the pointer to the first possible address byte (right after its header) in memory pool is returned.
 */
void *ma_malloc(size tsize) {

    //TODO: add your code here
    mem_chunk_header* HeaderPointer = (mem_chunk_header*)&mem_pool;
    int currentByte = 0;
    while(currentByte < MEM_POOL_SIZE){
        if(HeaderPointer->status == ALLOCATED){
            currentByte += HeaderPointer->size;
            HeaderPointer += currentByte;
            if(HeaderPointer->size == 0){
                HeaderPointer->status = FREE;
                HeaderPointer->size = MEM_POOL_SIZE - currentByte - sizeof(mem_chunk_header);
            }
        }
        else{
            if(HeaderPointer->size < tsize){
                return NULL;
            }
            else{
                HeaderPointer->size = tsize;
                HeaderPointer->status = ALLOCATED;
                return HeaderPointer + sizeof(mem_chunk_header);
            }
        }
    }
    return NULL;

}

/**
 * Releasing the bytes in memory pool which was hold by ptr, meaning makes those bytes available for other uses.
 * Implement also the coalescing behavior.
 */
void ma_free(void *ptr) {

    //TODO: add your code here

}

/**
 * This function is only for debugging. It prints out the entire memory pool.
 * Print info on headers and content of all elements in your pool
 */
void ma_print(void) {

    //TODO: add your code here

}
 
  
