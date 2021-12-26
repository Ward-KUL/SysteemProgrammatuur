#include "sbuffer.h"
#include "config.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

struct mutex_data{
    sbuffer_t* buffer;
    pthread_mutex_t lock;
};

mutex_data_t* get_mutex_data(){
    mutex_data_t* mut = malloc(sizeof(mutex_data_t));
    sbuffer_t* buffer;
    if(sbuffer_init(&buffer) != 0){
        printf("failed to initialize the buffer\n");
        exit(EXIT_FAILURE);
    }
    mut->buffer = buffer;
    pthread_mutex_init(&(mut->lock),NULL);
    return mut;
}

sensor_data_t* convert_sensor(sensor_data_packed_t orig){
    sensor_data_t* converted = malloc(sizeof(sensor_data_t));
    converted->id = orig.id;
    converted->ts = orig.ts;
    converted->value = orig.value;
    return converted;
}

void write_file(sbuffer_t* buffer){
    FILE* file = fopen("sensor_data","r");
    sensor_data_packed_t data_formatted;
    while(fread(&data_formatted,sizeof(sensor_data_packed_t),1,file)>0){
        sbuffer_insert(buffer,convert_sensor(data_formatted));
    }
    return;
}

void* get_succes_code(){
    int* error_code = malloc(sizeof(int));
    *error_code  = 0;
    return (void*)error_code;
}



void *writer_start_routine(void *arg){
    printf("writer routine called\n");
    mutex_data_t* mut = arg;
    write_file(mut->buffer);
    return get_succes_code();
}

void read_from_buffer(){
    
}

void *slow_reader_routine(void *arg){
    printf("slow routine called\n");
    
    return get_succes_code();
}

void *fast_reader_routine(void *arg){
    
    return get_succes_code();
}

void start_threads(){
    printf("Trying to start threads\n");
    pthread_t writer,reader_slow,reader_fast;
    mutex_data_t* mut = get_mutex_data();
    void* buffer = mut->buffer;
    pthread_create(&writer,NULL,writer_start_routine,buffer);
    pthread_create(&reader_slow,NULL,slow_reader_routine,buffer);
    pthread_create(&reader_fast,NULL,fast_reader_routine,buffer);
    pthread_join(writer,NULL);
    pthread_join(reader_fast,NULL);
    pthread_join(reader_slow,NULL);
    sbuffer_free(mut->buffer);
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

//een mutex lock op de bool of het al gelezen is of niet zodat ze niet tegelijk schrijven naar de bool
//als ze dan tegelijk lezen, geen probleem zolang ze maar niet tegelijk proberen de bool aan te passen