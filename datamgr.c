#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "datamgr.h"
#include "config.h"
#include "lib/dplist.h"


typedef dplist_t average_data_t;


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
// void* node_copy(void* element);
void node_free(void** node);
int node_compare(void * x, void * y);
void int_free(void** element);
void* int_copy(void* element);
int int_compare(void* x, void* y);
void print_sensor(sensor_data_t* sensor);
sensor_data_t* convert_packed(sensor_data_packed_t* p);
void parse_line_to_sensor_node(char* line);
void add_time_to_sensor(dplist_t*);
void add_new_measurement_to_average(sensor_node_t* node,sensor_data_t* data);
sensor_node_t* find_sensor_id(sensor_id_t id);

//testing function
void printSensorElements(dplist_t* list){
    int index = 0;
    while(dpl_size(list)>index){
        sensor_data_t* data = dpl_get_element_at_index(list,index);
        printf("sensor id %d, sensor value %f\n", data->id,data->value);
        index ++;
    }
}


void * element_copy(void * element) {
    sensor_data_t* copy = malloc(sizeof (sensor_data_t));
    assert(copy != NULL);
    copy->id = ((sensor_data_t*)element)->id;
    copy->ts = ((sensor_data_t*)element)->ts;
    copy->value = ((sensor_data_t*)element)->value;
    return (void *) copy;
}

// void* node_copy(void* element){
//     sensor_node_t* copy = malloc(sizeof(sensor_node_t));
//     assert(copy!=NULL);
//     copy->average_data = ((sensor_node_t*)element)->
// }

void element_free(void ** element) {
    //printf("This should be the element: %c\n",*(((my_element_t*)*element))->name);
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
    dplist_t* avg_data = ((sensor_node_t*)*node)->average_data;
    dpl_free(&avg_data,true);
    free(*node);
    *node = NULL;

}

int node_compare(void * x, void * y) {
    return ((((sensor_node_t*)x)->id_sensor < ((sensor_node_t*)y)->id_sensor) ? -1 : (((sensor_node_t*)x)->id_sensor == ((sensor_node_t*)y)->id_sensor) ? 0 : 1);
}



void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data){

    if(node_list != NULL) datamgr_free();//als er nog sensors in de lijst zitten eerst de lijst freeen zodat we geen memory leaks hebben
    //intialize the list
    dplist_t* data_list = dpl_create(element_copy,element_free,element_compare);
    node_list = dpl_create(NULL,node_free,node_compare);
    //create the list with the sensors
    sensor_node_t* node = malloc(sizeof(sensor_node_t));
    while(fscanf(fp_sensor_map,"%hd %hd",&(node->id_room),&(node->id_sensor))>0){
        average_data_t* average_data = dpl_create(int_copy,int_free,int_compare);
        (node->average_data) = average_data;
        dpl_insert_sorted(node_list,node,false);
        node = malloc(sizeof(sensor_node_t));
    }
    free(node);
    fclose(fp_sensor_map);
    //create the list with the data from the sensors
    sensor_data_packed_t data_formatted;
    while(fread(&data_formatted,sizeof(sensor_data_packed_t),1,fp_sensor_data)>0){
        sensor_data_t* data = convert_packed(&data_formatted);
        //print_sensor(data);
        if((SET_MAX_TEMP >= (data->value)&& ((data->value) <= SET_MIN_TEMP)))
            dpl_insert_sorted(data_list,data,true);
        free(data);

    }
    fclose(fp_sensor_data);
    //printSensorElements(data_list);
    add_time_to_sensor(data_list);
    return;   

}

void add_time_to_sensor(dplist_t* data_list){
    //loop through data list and remove one by one untill all data has been put inside the sensors
    char node_index = 0;
    int size  = dpl_size(node_list);
    while(node_index<size){
        sensor_node_t* node = dpl_get_element_at_index(node_list,node_index);
        sensor_data_t* data = dpl_get_element_at_index(data_list,0);
        while((dpl_get_first_reference(data_list) != NULL) && (((data = dpl_get_element_at_index(data_list,0))->id) == node->id_sensor)){
            add_new_measurement_to_average(node,data);
            if(node->last_modified>data->ts) node->last_modified = data->ts;
            dpl_remove_element(data_list,data,true);
        }
        node_index ++;
                                            //printf("%f\n",*(sensor_value_t*)dpl_get_element_at_index(node->average_data,1));
    }
    dpl_free(&data_list,true);
}

void add_new_measurement_to_average(sensor_node_t* node,sensor_data_t* data){
    if(dpl_size(node->average_data)>=RUN_AVG_LENGTH){
        //er moet er eerst ene weg voor we er een kunnen bijsteken
        dpl_remove_at_index(node->average_data,0,true);
    }
    sensor_value_t* new_value = malloc(sizeof(sensor_value_t));
    *new_value = data->value;
    dpl_insert_at_index(node->average_data,new_value,99,false);
    //printf("%f\n",*(sensor_value_t*)dpl_get_element_at_index(node->average_data,0));
    //printf("%d\n",dpl_size(node->average_data));
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
    dplist_t* list = node->average_data;
    char index = 0;
    int size = dpl_size(list);
    if(size<RUN_AVG_LENGTH) return 0;
    int sum = 0;
    while(index<size){
        sum += *((sensor_value_t*)dpl_get_element_at_index(list,index));
        index ++;
    }
    return sum/size;
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

