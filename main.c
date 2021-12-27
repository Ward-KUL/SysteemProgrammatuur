#include "sbuffer.h"
#include "config.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>


typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;         /**< a structure containing the data */
    bool has_been_read;         /** boolean that will be set to true once the data has been written*/
} sbuffer_node_t;


// mutex_data_t* get_mutex_data(){
//     mutex_data_t* mut = malloc(sizeof(mutex_data_t));
//     sbuffer_t* buffer;
//     if(sbuffer_init(&buffer) != 0){
//         printf("failed to initialize the buffer\n");
//         exit(EXIT_FAILURE);
//     }
//     mut->buffer = buffer;
//     pthread_mutex_init(&(mut->lock),NULL);
//     return mut;
// }

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
        sensor_data_t* data = convert_sensor(data_formatted);
        sbuffer_insert(buffer,data);
        free(data);
        printf("writer: data is:  sensor_id: %d, ts: %ld, value %f\n",data_formatted.id,data_formatted.ts,data_formatted.value);

    }
    fclose(file);
    if(sbuffer_done_writing(buffer)!= SBUFFER_SUCCESS){
        printf("Couldn't stop the writing process of the buffer\n");
        return;
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
    sbuffer_t* buffer = arg;
    write_file(buffer);
    return get_succes_code();
}

// sensor_data_t* read_from_buffer(sbuffer_t* buffer){
//     sensor_data_t* data = malloc(sizeof(sensor_data_t));
//     sbuffer_node_t** node = NULL;
//     int res = sbuffer_read_and_remove(buffer,data,node);
//     if(res != SBUFFER_SUCCESS){
//         if(res == SBUFFER_NO_DATA)
//             usleep(10000);//sleep for 10 ms
//         else
//             printf("Failure reading from buffer\n");
//     }
//     return data;
// }

void *slow_reader_routine(void *arg){
    printf("slow routine called\n");
    sensor_data_t* data = malloc(sizeof(sensor_data_t));
    sbuffer_node_t** node = malloc(sizeof(sbuffer_node_t*));
    sbuffer_t* buffer = arg;
    while(1){
        // sleep(1);
        int res = sbuffer_read_and_remove(buffer,data,node);
        if(res != SBUFFER_SUCCESS){
            if(res == SBUFFER_NO_DATA){
                if(sbuffer_is_buffer_done_writing(buffer) == false)
                    usleep(100000);//sleep for 10 ms
                else{//there won't be anymore data added
                    free(node);
                    free(data);
                    break;
                }
            }
            else
                printf("Failure reading from buffer\n");
        }
        else{
            printf("reader 1: data is:  sensor_id: %d, ts: %ld, value %f\n",data->id,data->ts,data->value);

        }
    }
    return get_succes_code();
}

void *fast_reader_routine(void *arg){
    printf("slow routine called\n");
    sensor_data_t* data = malloc(sizeof(sensor_data_t));
    sbuffer_node_t** node = malloc(sizeof(sbuffer_node_t*));
    sbuffer_t* buffer = arg;
    while(1){
        // sleep(1);
        int res = sbuffer_read_and_remove(buffer,data,node);
        if(res != SBUFFER_SUCCESS){
            if(res == SBUFFER_NO_DATA){
                if(sbuffer_is_buffer_done_writing(buffer) == false)
                    usleep(100000);//sleep for 10 ms
                else{//there won't be anymore data added
                    free(node);
                    free(data);
                    break;
                }
            }
            else
                printf("Failure reading from buffer\n");
        }
        else{
            printf("reader 2: data is:  sensor_id: %d, ts: %ld, value %f\n",data->id,data->ts,data->value);

        }
    }
    return get_succes_code();
}

void start_threads(){
    printf("Trying to start threads\n");
    pthread_t writer,reader_slow,reader_fast;
    sbuffer_t* buffer;
    if(sbuffer_init(&buffer) != 0){
        printf("failed to initialize the buffer\n");
        exit(EXIT_FAILURE);
    }
    void *writer_result = NULL;
    void *reader_slow_result = NULL;
    void *reader_fast_result = NULL;
    pthread_create(&writer,NULL,writer_start_routine,buffer);
    pthread_create(&reader_slow,NULL,slow_reader_routine,buffer);
    pthread_create(&reader_fast,NULL,fast_reader_routine,buffer);
    pthread_join(writer,&writer_result);
    pthread_join(reader_fast,&reader_fast_result);
    pthread_join(reader_slow,&reader_slow_result);
    sbuffer_free(&buffer);
    free(writer_result);
    free(reader_fast_result);
    free(reader_slow_result);
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