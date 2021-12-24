#include "sbuffer.h"
#include "config.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

struct flags{
    bool writer_busy;
    bool reader_slow_busy;
    bool reader_fast_busy;
};

void* get_flags(){
    flags_t* flags = malloc(sizeof(flags_t));
    flags->writer_busy = false;
    flags->reader_fast_busy = false;
    flags->reader_slow_busy = false;
    return (void*)flags;
}



void *writer_start_routine(void *arg){
    printf("writer routine called\n");
    flags_t* flags = arg;
    printf("busy\n");
    flags->writer_busy = true;
    sleep(3);
    flags->writer_busy = false;
    return NULL;
}

void *slow_reader_routine(void *arg){
    printf("slow routine called\n");
    flags_t* flags = arg;
    sleep(1);
    while(flags->writer_busy == true){
        //nop
    }
    printf("not busy anymore\n");
    return NULL;
}

void *fast_reader_routine(void *arg){
    printf("fast routine called\n");
    return NULL;
}

void start_threads(){
    printf("Trying to start threads\n");
    pthread_t writer,reader_slow,reader_fast;
    void* flags = get_flags();
    pthread_create(&writer,NULL,writer_start_routine,flags);
    pthread_create(&reader_slow,NULL,slow_reader_routine,flags);
    pthread_create(&reader_fast,NULL,fast_reader_routine,flags);
    pthread_join(writer,NULL);
    pthread_join(reader_fast,NULL);
    pthread_join(reader_slow,NULL);
}

int main(void){
    start_threads();
    return 0;
}

//Huidige strategie:
//Ge maakt in de start threads functie de buffer aan die geeft ge mee zoals dat ge de flags 
//al hebt meegegeven. Dan steekt ge daar de shit in met de writer en leest ge het eruit met
//de reader. Ge kunt wrs wel werken met de flags maar ik vermoed dat als ge eens kijkt naar 
//de andere soorten die in de labotekst staan dat ge dan wat meer uitleg zult vinden.
//en opzich dan zou het niet zo heel moeilijk meer moeten zijn