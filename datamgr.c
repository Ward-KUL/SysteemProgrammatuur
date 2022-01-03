#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "datamgr.h"
#include "config.h"
#include "lib/dplist.h"


typedef struct{
    sensor_value_t run_avg[RUN_AVG_LENGTH];
    int lastPos;
    sensor_value_t previous_avg;
}average_data_t;


/**
 * structure to hold sensornode data
 */
typedef struct{
    sensor_id_t id_sensor;
    room_id_t id_room;
    average_data_t* average_data;
    sensor_ts_t last_modified;
}sensor_node_t;

dplist_t* node_list;


void* element_copy(void * element);
void element_free(void ** element);
int element_compare(void * x, void * y);
void node_free(void** node);
int node_compare(void * x, void * y);
void int_free(void** element);
void* int_copy(void* element);
int int_compare(void* x, void* y);
void print_sensor(sensor_data_t* sensor);
sensor_data_t* convert_packed(sensor_data_packed_t* p);
void parse_line_to_sensor_node(char* line);
sensor_node_t* find_sensor_id(sensor_id_t id);
size_t get_size_of_array(sensor_value_t* array);

void * element_copy(void * element) {
    sensor_data_t* copy = malloc(sizeof (sensor_data_t));
    assert(copy != NULL);
    copy->id = ((sensor_data_t*)element)->id;
    copy->ts = ((sensor_data_t*)element)->ts;
    copy->value = ((sensor_data_t*)element)->value;
    return (void *) copy;
}

void element_free(void ** element) {
    free(((sensor_node_t*)element)->average_data);
    free(*element);
    *element = NULL;
}

int element_compare(void * x, void * y) {
    return ((((sensor_data_t*)x)->id < ((sensor_data_t*)y)->id) ? -1 : (((sensor_data_t*)x)->id == ((sensor_data_t*)y)->id) ? 0 : 1);
}

void int_free(void** element){
    free(*element);
    *element = NULL;
    return;
}
void* int_copy(void* element){
    sensor_data_t* copy = malloc(sizeof(sensor_data_t));
    assert(copy!=NULL);
    return (void*) copy;
}

int int_compare(void* x, void* y){
    return (x<y ? -1: x == y ? 0: 1);
}

void node_free(void** node){
    average_data_t* run_avg = ((sensor_node_t*)*node)->average_data;
    free(run_avg);
    free(*node);
    *node = NULL;

}

int node_compare(void * x, void * y) {
    return ((((sensor_node_t*)x)->id_sensor < ((sensor_node_t*)y)->id_sensor) ? -1 : (((sensor_node_t*)x)->id_sensor == ((sensor_node_t*)y)->id_sensor) ? 0 : 1);
}

void add_new_measurement_to_average(sensor_node_t* node,sensor_value_t new_value){
    node->average_data->run_avg[node->average_data->lastPos] = new_value;
    node->average_data->lastPos ++;
    if(node->average_data->lastPos>=RUN_AVG_LENGTH) node->average_data->lastPos =0; //overflow
    if(get_size_of_array(node->average_data->run_avg) >= RUN_AVG_LENGTH){    
        //calculate run_avg
        sensor_value_t sum = 0;
        for(int i = 0;i < RUN_AVG_LENGTH;i ++){
            sum += node->average_data->run_avg[i];
        }
        sum = sum/RUN_AVG_LENGTH;
        char* buffer;
        if(SET_MAX_TEMP < sum){
            asprintf(&buffer,"The sensor node with %d reports it's too hot(running avg temperature = %f)",node->id_sensor,sum);
            write_to_logger(buffer);
            free(buffer);
        }
        else if(sum < SET_MIN_TEMP){
            asprintf(&buffer,"The sensor node with %d reports it's too cold(running avg temperature = %f)",node->id_sensor,sum);
            write_to_logger(buffer);
            free(buffer);
        }
        node->average_data->previous_avg = sum;
    }
}

size_t get_size_of_array(sensor_value_t* array){
    if(array[0] < -272) return 0;
    for(size_t i = 0;i<RUN_AVG_LENGTH; i++){
        if(array[i]<-273){
            //temperature hasn't been changed by a sensor
            return i;
        }
    }
    
    return (RUN_AVG_LENGTH);
}

average_data_t* get_default_avg(){
    average_data_t* average_data = malloc(sizeof(average_data_t));
    average_data->lastPos = 0;
    average_data->previous_avg = 0;
    for(int i = 0;i<RUN_AVG_LENGTH;i++){
        average_data->run_avg[i] = -274; //lowest possible temperature which should not be recorded
    }
    return average_data;
}

int datamgr_add_new_sensor_data(sensor_data_packed_t data){
    sensor_node_t* sensor = find_sensor_id(data.id);
    if(sensor == NULL){
        //sensor was not found
        printf("Sensor was not found\n");
        return -1;
    }
    add_new_measurement_to_average(sensor,data.value);
    if(sensor->last_modified<data.ts) sensor->last_modified = data.ts;
    return 0;
}


void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data){

    if(node_list != NULL) datamgr_free();//als er nog sensors in de lijst zitten eerst de lijst freeen zodat we geen memory leaks hebben
    //intialize the list
    node_list = dpl_create(NULL,node_free,node_compare);
    //create the list with the sensors
    sensor_node_t* node = malloc(sizeof(sensor_node_t));
    while(fscanf(fp_sensor_map,"%hd %hd",&(node->id_room),&(node->id_sensor))>0){
        node->average_data = get_default_avg();
        node->last_modified = 0;
        dpl_insert_sorted(node_list,node,false);
        node = malloc(sizeof(sensor_node_t));
    }
    free(node);
    
    //create the list with the data from the sensors
    if(fp_sensor_data != NULL){
        sensor_data_packed_t data_formatted;
        while(fread(&data_formatted,sizeof(sensor_data_packed_t),1,fp_sensor_data)>0){
            datamgr_add_new_sensor_data(data_formatted);
        }
    }
    return;   

}

void datamgr_free(){
    dpl_free(&node_list,true);
    node_list = NULL;
}
uint16_t datamgr_get_room_id(sensor_id_t sensor_id){
    if(node_list == NULL) return -1;
    sensor_node_t* node = find_sensor_id(sensor_id);
    ERROR_HANDLER(node == NULL,"Could not find the sensor id in the datamg");
    return node->id_room;
}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id){
    if(node_list == NULL) return -1;
    sensor_node_t* node = find_sensor_id(sensor_id);
    ERROR_HANDLER(node == NULL,"Could not find the sensor id in the datamgr");
    return node->average_data->previous_avg;
}

time_t datamgr_get_last_modified(sensor_id_t sensor_id){
    if(node_list == NULL) return -1;
    sensor_node_t* node = find_sensor_id(sensor_id);
    ERROR_HANDLER(node == NULL,"Could not find the sensor id in the datamg");
    return node->last_modified; 
}

int datamgr_get_total_sensors(){
    return dpl_size(node_list);
}

//function that returns the sensor with the corresponding id from the internal list, returns NULL of sensor not found
sensor_node_t* find_sensor_id(sensor_id_t id){
    int index = 0;
    int size = dpl_size(node_list);
    while(index<size){
        sensor_node_t* data = dpl_get_element_at_index(node_list,index);
        if(data->id_sensor == id) return data;
        index ++;
    }
    return NULL;
}

void print_sensor(sensor_data_t* sensor){
    printf("The sensor contains: id = %d, value = %f, timestamp = %ld (print statement was written on line) %d\n",sensor->id,sensor->value,sensor->ts,__LINE__);
    return;
}

sensor_data_t* convert_packed(sensor_data_packed_t* p){
    sensor_data_t* d = malloc(sizeof(sensor_data_t));
    d->id = p->id;
    d->ts = p->ts;
    d->value = p->value;
    return d;
}

