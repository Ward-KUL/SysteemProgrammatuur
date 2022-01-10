/**
 * \author Ward Smets
 */

#ifndef DATAMGR_H_
#define DATAMGR_H_

#include <stdlib.h>
#include <stdio.h>
#include "config.h"

#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5
#endif

#ifndef SET_MAX_TEMP
#error SET_MAX_TEMP not set
#endif

#ifndef SET_MIN_TEMP
#error SET_MIN_TEMP not set
#endif



/**
 * 
 *  Starts datamgr and parses the room,sensor map
 *  \param fp_sensor_map file pointer to the map file
 */
void datamgr_parse_sensor_map(FILE *fp_sensor_map);

/**
 * This method should be called to clean up the datamgr, and to free all used memory. 
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors will not return a valid result
 */
void datamgr_free();

/**
 * @brief use to add new sensor_data to the data manager, if the sensor has not yet been added.
 * 
 * @param data the data of the sensor that is to be added
 */
int datamgr_add_new_sensor_data(sensor_data_packed_t data);

#endif  //DATAMGR_H_
