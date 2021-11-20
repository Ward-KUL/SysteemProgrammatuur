#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "datamgr.h"
#include "config.h"
#include "lib/dplist.h"


dplist_t* data_list;
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
void add_time_to_sensor(void);
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
    free(((sensor_node_t*)*node)->average_data);
    free(*node);
    *node = NULL;

}

int node_compare(void * x, void * y) {
    return ((((sensor_node_t*)x)->id_sensor < ((sensor_node_t*)y)->id_sensor) ? -1 : (((sensor_node_t*)x)->id_sensor == ((sensor_node_t*)y)->id_sensor) ? 0 : 1);
}



void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data){

    //intialize the list
    data_list = dpl_create(element_copy,element_free,element_compare);
    node_list = dpl_create(NULL,node_free,node_compare);

    char* line = NULL;
    size_t len = 0;
    while(getline(&line,&len,fp_sensor_map) != -1){
        parse_line_to_sensor_node(line);
    }
    fclose(fp_sensor_map);

    sensor_data_packed_t data_formatted;
    while(fread(&data_formatted,sizeof(sensor_data_packed_t),1,fp_sensor_data)>0){
        sensor_data_t* data = convert_packed(&data_formatted);
        //print_sensor(data);
        dpl_insert_sorted(data_list,data,true);
        free(data);

    }
    fclose(fp_sensor_data);
    //printSensorElements(data_list);
    add_time_to_sensor();
    return;   

}

void add_time_to_sensor(void){
    //loop through data list and remove one by one untill all data has been put inside the sensors
    char node_index = 0;
    int size  = dpl_size(node_list);
    while(node_index<size){
        //printf("Big loop\n %d",node_index);
        sensor_node_t* node = dpl_get_element_at_index(node_list,node_index);
        sensor_data_t* data;
        while((dpl_get_first_reference(data_list) != NULL) && (((data = dpl_get_element_at_index(data_list,0))->id) == node->id_sensor)){
            //printf("%d\n",node_index);
            add_new_measurement_to_average(node,data);
            if(node->last_modified>data->ts) node->last_modified = data->ts;
            dpl_remove_element(data_list,data,true);
        }
        node_index ++;
    }
    dpl_free(&data_list,true);
}

void add_new_measurement_to_average(sensor_node_t* node,sensor_data_t* data){
    if(dpl_size(node->average_data)>=RUN_AVG_LENGTH){
        //er moet er eerst ene weg voor we er een kunnen bijsteken
        dpl_remove_at_index(node->average_data,0,false);
    }
    dpl_insert_at_index(node->average_data,&(data->value),99,true);
}


void parse_line_to_sensor_node(char* line){
    sensor_node_t* node = malloc(sizeof(sensor_node_t));
    sscanf(line,"%hd %hd",&(node->id_room),&(node->id_sensor));
    average_data_t* average_data = dpl_create(int_copy,int_free,int_compare);
    (node->average_data) = average_data;
    dpl_insert_sorted(node_list,node,false);
    return;
}

void datamgr_free(){
    dpl_free(&data_list,true);
}
uint16_t datamgr_get_room_id(sensor_id_t sensor_id){
    sensor_node_t* node = find_sensor_id(sensor_id);
    ERROR_HANDLER(node == NULL,"Could not find the sensor id in the datamg");
    return node->id_room;
}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id){
    sensor_node_t* node = find_sensor_id(sensor_id);
    ERROR_HANDLER(node == NULL,"Could not find the sensor id in the datamgr");
    dplist_t* list = node->average_data;
    char index = 0;
    int size = dpl_size(list);
    int sum = 0;
    while(index<size){
        sum += *((int*)dpl_get_element_at_index(list,index));
    }
    return sum;
}

sensor_node_t* find_sensor_id(sensor_id_t id){
    int index = 0;
    int size = dpl_size(node_list);
    while(index<size){
        sensor_node_t* data = dpl_get_element_at_index(data_list,index);
        if(data->id_sensor == id) return data;
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