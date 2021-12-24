#include "sbuffer.h"
#include "config.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>






void *writer_start_routine(void *arg){
    printf("writer routine called\n");
    return NULL;
}

void *slow_reader_routine(void *arg){
    printf("slow routine called\n");
    return NULL;
}

void *fast_reader_routine(void *arg){
    printf("fast routine called\n");
    return NULL;
}

void start_threads(){
    printf("Trying to start threads\n");
    pthread_t writer,reader_slow,reader_fast;
    pthread_create(&writer,NULL,writer_start_routine,NULL);
    pthread_create(&reader_slow,NULL,slow_reader_routine,NULL);
    pthread_create(&reader_fast,NULL,fast_reader_routine,NULL);
    pthread_join(writer,NULL);
    pthread_join(reader_fast,NULL);
    pthread_join(reader_slow,NULL);
}

int main(void){
    start_threads();
    return 0;
}