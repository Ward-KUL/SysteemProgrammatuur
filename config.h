/**
 * \author Ward Smets
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <time.h>
#include "lib/dplist.h"

typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine
typedef uint16_t room_id_t;
typedef dplist_t average_data_t;

/**
 * structure to hold sensor data
 */
typedef struct {
    sensor_id_t id;         /** < sensor id */
    sensor_value_t value;   /** < sensor value */
    sensor_ts_t ts;         /** < sensor timestamp */
} sensor_data_t;

/**
 * structure to hold sensor data_packed
 */
typedef struct {
    sensor_id_t id __attribute__((packed));         /** < sensor id */
    sensor_value_t value __attribute__((packed));   /** < sensor value */
    sensor_ts_t ts __attribute__((packed));    /** < sensor timestamp */
} sensor_data_packed_t;



/**
 * structure to hold sensornode data
 */
typedef struct{
    sensor_id_t id_sensor;
    room_id_t id_room;
    average_data_t* average_data;
    sensor_ts_t last_modified;
}sensor_node_t;



#endif /* _CONFIG_H_ */
