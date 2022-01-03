#define _DEFAULT_SOURCE//for usleep
#define _GNU_SOURCE

#include "sbuffer.h"
#include "config.h"
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "datamgr.h"
#include "sensor_db.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "connmgr.h"

typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;         /**< a structure containing the data */
    bool has_been_read;         /** boolean that will be set to true once the data has been written*/
} sbuffer_node_t;

char* fifo_exit_code = "Close fifo code: 1@3ks93k4j32";
pthread_mutex_t lock;
FILE* fifo_descr_wr = NULL;


void write_to_logger(char* to_write){
    if(fifo_descr_wr == NULL){
        printf("Couldn't write to fifo because pointer is NULL\n");
        exit(EXIT_FAILURE);
    }
    char *send_buf;
    asprintf(&send_buf,"%s\n",to_write);
    // pthread_mutex_lock(&lock);
    if(fputs(send_buf,fifo_descr_wr) == EOF){
        printf("Failed to write to fifo\n");
    }
    // pthread_mutex_unlock(&lock);
    printf("Tried to write the following to the logger %s",send_buf);
    free(send_buf);
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
        sensor_data_t* data = convert_sensor(data_formatted);
        sbuffer_insert(buffer,data);
        free(data);
        // printf("writer: data is:  sensor_id: %d, ts: %ld, value %f\n",data_formatted.id,data_formatted.ts,data_formatted.value);

    }
    fclose(file);
    if(sbuffer_done_writing(buffer)!= SBUFFER_SUCCESS){
        printf("Couldn't stop the writing process of the buffer\n");
        return;
        
    }
    return;
}



void *writer_start_routine(void *arg){
    printf("writer routine called\n");
    sbuffer_t* buffer = arg;
    //start tcp_listener
    connmgr_listen(5678,buffer);
    connmgr_free();
    return NULL;
}


void *slow_reader_routine(void *arg){
    //start the db
    printf("slow routine called\n");
    DBCONN* conn = init_connection(1,fifo_exit_code,lock);
    sensor_data_t* data = malloc(sizeof(sensor_data_t));
    sbuffer_node_t** node = malloc(sizeof(sbuffer_node_t*));
    *node = NULL;
    sbuffer_t* buffer = arg;
    while(1){
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
            insert_sensor(conn,data->id,data->value,data->ts);
            // printf("reader 1: data is:  sensor_id: %d, ts: %ld, value %f\n",data->id,data->ts,data->value);

        }
        usleep(100);
    }
    disconnect(conn);
    printf("Database is done reading\n");
    return NULL;
}

void *fast_reader_routine(void *arg){
    printf("slow routine called\n");
    sensor_data_t* data = malloc(sizeof(sensor_data_t));
    sensor_data_packed_t* data_packed = malloc(sizeof(sensor_data_packed_t));
    sbuffer_node_t** node = malloc(sizeof(sbuffer_node_t*));
    *node = NULL;
    sbuffer_t* buffer = arg;

    FILE* sensor_map = fopen("room_sensor.map","r");
    datamgr_parse_sensor_files(sensor_map,NULL);//we start with only the sensor and there according rooms
    fclose(sensor_map);
    while(1){
        int res = sbuffer_read_and_remove(buffer,data,node);
        if(res != SBUFFER_SUCCESS){
            if(res == SBUFFER_NO_DATA){
                if(sbuffer_is_buffer_done_writing(buffer) == false)
                    usleep(100000);//sleep for 10 ms
                else{//there won't be anymore data added
                    free(node);
                    free(data);
                    free(data_packed);
                    break;
                }
            }
            else
                printf("Failure reading from buffer\n");
        }
        else{
            data_packed->id = data->id;
            data_packed->ts = data->ts;
            data_packed->value = data->value;
            datamgr_add_new_sensor_data(*data_packed);
            // printf("reader 2: data is:  sensor_id: %d, ts: %ld, value %f\n",data->id,data->ts,data->value);

        }
        usleep(10);
    }
    datamgr_free();
    return NULL;
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
    // free(writer_result);
    // free(reader_fast_result);
    // free(reader_slow_result);
}

void start_logger(FILE* fifo){
    printf("Logger started\n");
    int log_count = 0;
    int MAX_BUFFER_SIZE = 150;
    char receive_buffer[MAX_BUFFER_SIZE];//set in header file with define
    char* str_result = NULL;
    time_t time_v;
    FILE* gateway = fopen("gateway.log","w");
    do{       
        // pthread_mutex_lock(&mutex_lock);
        str_result = fgets(receive_buffer,MAX_BUFFER_SIZE,fifo);
        // pthread_mutex_unlock(&mutex_lock);
        if(str_result != NULL){
             printf("Received the following : %s",str_result);
            //received something
            str_result[strcspn(str_result, "\n")] = 0;//haal de newline character van str_result
            time(&time_v);//set timer variable to current time
            if(fprintf(gateway,"<%d> <%ld> <%s>\n",log_count,time_v,str_result)< 0){
                printf("Couldn't write to the gateway log file but we did receive something\n");
                exit(EXIT_FAILURE);
            }
            log_count++;
        }
    }
    while(strcmp(str_result,fifo_exit_code) != 0);
    printf("logger closes\n");
    //done receiving, close everything
    if(fclose(fifo) != 0){
        printf("Logger couldn't close fifo\n");
    }
    fclose(gateway);
    waitpid(getppid(),0,0);
    printf("Everything synced up and closed\n");
    exit(EXIT_SUCCESS);

}

int main(void){

    char* fifo_path = "path_to_fifo";
    if(mkfifo(fifo_path,0666) != 0){
        if(errno != EEXIST){
            printf("Failed to create fifo and it doesn't exist already either\n");
            exit(EXIT_FAILURE);
        }
    }
    pid_t childPid = fork();
    if(childPid == 0){
        //child process
        FILE* fifo = fopen(fifo_path,"r");
        if(fifo == NULL){
            printf("Failed to open the fifo to write data to it\n");
            exit(EXIT_FAILURE);
        }
        start_logger(fifo);
        printf("logging process finished\n");
    }
    else{
        //parent
        fifo_descr_wr = fopen(fifo_path,"w");
        if(fifo_descr_wr == NULL){
            printf("Failed to open the fifo to write data to it\n");
            exit(EXIT_FAILURE);
        }
        start_threads();    
        printf("main process finished \n");
        exit(EXIT_SUCCESS);
    }
    // int returncode;
    // waitpid(childPid,&returncode,0);
    






    // char* fifo_path = "fifo_path";
    // if(mkfifo(fifo_path,0666)!= 0){
    //     if(errno != EEXIST){
    //         printf("Failed to create fifo and it doesn't exist already either\n");
    //         exit(EXIT_FAILURE);
    //     }
    //     else{
    //         printf("fifo didn't need to be created since it already existed\n");
    //     }
    // }
    // //mutex_lock is globally defined
    // pthread_mutex_init(&lock,NULL);

    // pid_t childPid = fork();
    // if(childPid == 0){
    //     //child process -> start the logger
    //     FILE* fifo_read = fopen(fifo_path,"r");
    //     if(fifo_read == NULL){
    //         printf("Couldn't open the fifo to read from it\n");
    //         exit(EXIT_FAILURE);
    //     }
    //     start_logger(fifo_read);
    // }
    // else{
    //     //parent process -> start the threads
    //     fifo_descr_wr = fopen(fifo_path,"w");
    //     if( fifo_descr_wr == NULL){
    //         printf("Couldn't open the fifo to write to it\n");
    //         exit(EXIT_FAILURE);
    //     }
    //     write_to_logger("plz man");
    //     start_threads();
    // }
    // int exit_code;
    // waitpid(childPid,&exit_code,0);
    // printf("Everything synced up and closing");    
    return 0;
}