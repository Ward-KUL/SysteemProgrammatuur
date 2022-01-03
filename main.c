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

/**
 * container object that gives arguments to the thread routines
 */
typedef struct argument_thread{
    sbuffer_t* buffer;
    int port_nr;
    char* fifo_exit_code;
}argument_thread_t;

/**
 * Global variables
 */
pthread_mutex_t lock;
FILE* fifo_descr_wr = NULL;


void write_to_logger(char* to_write){
    DEBUG_PRINTF("LOGGER: %s\n",to_write);
    ERROR_HANDLER(fifo_descr_wr == NULL, "Couldn't write to fifo because pointer is NULL");
    char *send_buf;
    asprintf(&send_buf,"%s\n",to_write);
    pthread_mutex_lock(&lock);
    ERROR_HANDLER(fputs(send_buf,fifo_descr_wr) == EOF,"Failed to write to fifo");
    pthread_mutex_unlock(&lock);    
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
    ERROR_HANDLER(file == NULL,"Failed to open sensor_data");
    sensor_data_packed_t data_formatted;
    while(fread(&data_formatted,sizeof(sensor_data_packed_t),1,file)>0){
        sensor_data_t* data = convert_sensor(data_formatted);
        ERROR_HANDLER(sbuffer_insert(buffer,data)!=SBUFFER_SUCCESS,"Failed to write to buffer");
        free(data);
        DEBUG_PRINTF("inserted the following in the buffer:  sensor_id: %d, ts: %ld, value %f\n",data_formatted.id,data_formatted.ts,data_formatted.value);
    }
    fclose(file);
    ERROR_HANDLER(sbuffer_done_writing(buffer)!= SBUFFER_SUCCESS,"Couldn't stop the writing process of the buffer");
    return;
}


void *tcp_listener_routine(void *arg){
    DEBUG_PRINTF("writer routine called\n");
    argument_thread_t* arguments = arg;
    sbuffer_t* buffer = arguments->buffer;
    connmgr_listen(arguments->port_nr,buffer);
    connmgr_free();
    write_to_logger(arguments->fifo_exit_code);
    return NULL;
}


void *database_reader_routine(void *arg){
    DEBUG_PRINTF("slow routine called\n");
    DBCONN* conn = init_connection(1);
    ERROR_HANDLER(conn == NULL,"Failed to open the database");
    sensor_data_t* data = malloc(sizeof(sensor_data_t));
    sbuffer_node_t** node = malloc(sizeof(sbuffer_node_t*));
    *node = NULL;
    argument_thread_t* arguments = arg;
    sbuffer_t* buffer = arguments->buffer;
    while(1){
        int res = sbuffer_read_and_remove(buffer,data,node);
        if(res != SBUFFER_SUCCESS){
            if(res == SBUFFER_NO_DATA){
                if(sbuffer_is_buffer_done_writing(buffer) == true){
                    //there won't be anymore data added
                    free(node);
                    free(data);
                    break;
                }
            }
            else
                DEBUG_PRINTF("Failure reading from buffer\n");
        }
        else{
            ERROR_HANDLER(insert_sensor(conn,data->id,data->value,data->ts)!=0,"Failed to insert new record");
            // DEBUG_PRINTF("database inserted the following data:  sensor_id: %d, ts: %ld, value %f\n",data->id,data->ts,data->value);
        }
    }
    disconnect(conn);
    DEBUG_PRINTF("Database is done reading\n");
    return NULL;
}

void *datamgr_reader_routine(void *arg){
    printf("fast routine called\n");
    sensor_data_t* data = malloc(sizeof(sensor_data_t));
    sensor_data_packed_t* data_packed = malloc(sizeof(sensor_data_packed_t));
    sbuffer_node_t** node = malloc(sizeof(sbuffer_node_t*));
    *node = NULL;
    argument_thread_t* arguments = arg;
    sbuffer_t* buffer = arguments->buffer;

    FILE* sensor_map = fopen("room_sensor.map","r");
    ERROR_HANDLER(sensor_map == NULL,"Failed to open room_sensor.map");
    datamgr_parse_sensor_files(sensor_map,NULL);//we start with only the sensor and there according rooms
    ERROR_HANDLER(fclose(sensor_map),"Failed to close file");
    while(1){
        int res = sbuffer_read_and_remove(buffer,data,node);
        if(res != SBUFFER_SUCCESS){
            if(res == SBUFFER_NO_DATA){
                if(sbuffer_is_buffer_done_writing(buffer) == true){//there won't be anymore data added
                    free(node);
                    free(data);
                    free(data_packed);
                    break;
                }
            }
            else
                DEBUG_PRINTF("Failure reading from buffer\n");
        }
        else{
            data_packed->id = data->id;
            data_packed->ts = data->ts;
            data_packed->value = data->value;
            ERROR_HANDLER(datamgr_add_new_sensor_data(*data_packed)!=0,"Failed to add data to the datamgr");
            // printf("reader 2: data is:  sensor_id: %d, ts: %ld, value %f\n",data->id,data->ts,data->value);

        }
    }
    datamgr_free();
    return NULL;
}

void start_threads(int port_nr,char* fifo_exit_code){
    printf("Trying to start threads\n");
    pthread_t writer,reader_slow,reader_fast;
    argument_thread_t* arguments = malloc(sizeof(argument_thread_t));
    sbuffer_t* buffer;
    ERROR_HANDLER(sbuffer_init(&buffer) != 0,"failed to initialize the buffer");
    arguments->buffer = buffer;
    arguments->port_nr = port_nr;
    arguments->fifo_exit_code = fifo_exit_code;

    ERROR_HANDLER(pthread_create(&writer,NULL,tcp_listener_routine,arguments)!=0,"Failed to create thread");
    ERROR_HANDLER(pthread_create(&reader_slow,NULL,database_reader_routine,arguments)!=0,"Failed to create thread");
    ERROR_HANDLER(pthread_create(&reader_fast,NULL,datamgr_reader_routine,arguments)!=0,"Failed to create thread");
    ERROR_HANDLER(pthread_join(writer,NULL)!=0,"Failed to join thread");
    ERROR_HANDLER(pthread_join(reader_fast,NULL)!=0,"Failed to join thread");
    ERROR_HANDLER(pthread_join(reader_slow,NULL)!=0,"Failed to join thread");
    ERROR_HANDLER(sbuffer_free(&buffer)!=SBUFFER_SUCCESS,"Failed to free buffer");
    free(arguments);
}

void start_logger(FILE* fifo,char* fifo_exit_code){
    DEBUG_PRINTF("Logger started\n");
    int log_count = 0;
    int MAX_BUFFER_SIZE = 150;
    char receive_buffer[MAX_BUFFER_SIZE];//set in header file with define
    char* str_result = NULL;
    time_t time_v;
    FILE* gateway = fopen("gateway.log","w");
    ERROR_HANDLER(gateway == NULL, "Failed to open gateway.log");
    do{       
        pthread_mutex_lock(&lock);
        str_result = fgets(receive_buffer,MAX_BUFFER_SIZE,fifo);
        pthread_mutex_unlock(&lock);
        if(str_result != NULL){
            //received something
            str_result[strcspn(str_result, "\n")] = 0;//remove the newline character from str_result
            time(&time_v);
            ERROR_HANDLER(fprintf(gateway,"<%d> <%ld> <%s>\n",log_count,time_v,str_result)< 0,"Couldn't write to the gateway log file but we did receive something\n");
            log_count++;
        }
    }
    while(strcmp(str_result,fifo_exit_code) != 0);
    DEBUG_PRINTF("logger closes\n");
    ERROR_HANDLER(fclose(fifo) != 0,"Logger couldn't close fifo");
    ERROR_HANDLER(fclose(gateway)!=0,"Logger couldn't close gateway");
    DEBUG_PRINTF("Everything synced up and closed\n");
    exit(EXIT_SUCCESS);

}

int main(int argc,char *argv[]){

    if(argc != 2){
        printf("FAILED TO START PROGRAM \n\n\nNeed the tcp port number\ne.g:./gateway 5678");
        exit(EXIT_SUCCESS);
    }
    int port_nr = atoi(argv[1]);
    //mutex lock is globally defined
    pthread_mutex_init(&lock,NULL);

    char* fifo_exit_code = "Close fifo code: 1@3ks93k4j32";
    char* fifo_path = "logFifo";
    if(mkfifo(fifo_path,0666) != 0){
        ERROR_HANDLER(errno != EEXIST,"Failed to create fifo and it doesn't exist already either");
    }
    pid_t childPid = fork();
    if(childPid == 0){
        //child process
        FILE* fifo = fopen(fifo_path,"r");
        ERROR_HANDLER(fifo == NULL,"Failed to open the fifo to write data to it\n");
        start_logger(fifo,fifo_exit_code);
    }
    else{
        //parent
        fifo_descr_wr = fopen(fifo_path,"w");
        ERROR_HANDLER(fifo_descr_wr == NULL,"Failed to open the fifo to write data to it\n");
        start_threads(port_nr,fifo_exit_code);   
        fclose(fifo_descr_wr); 
        DEBUG_PRINTF("main process finished \n");
        waitpid(childPid,0,0);
        exit(EXIT_SUCCESS); 
    }   
    return 0;
}