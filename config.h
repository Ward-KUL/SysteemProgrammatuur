/**
 * \author Ward Smets
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <time.h>

/**
 * error codes
 */
#define MEMORY_ERROR
#define FILE_ERROR

typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine
typedef struct mutex_data mutex_data_t;
typedef uint16_t room_id_t;


#ifdef DEBUG
#define DEBUG_PRINTF(...)                           \
            do{                                                                          \
            fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);     \
            fprintf(stderr,__VA_ARGS__);                                                \
            fflush(stderr);                                                             \
            }while(0)                      
#else
#define DEBUG_PRINTF(...)(void)0
#endif     

/*
 * Use ERROR_HANDLER() for handling memory allocation problems, invalid sensor IDs, non-existing files, etc.
 */
#define ERROR_HANDLER(condition, ...)    \
                    do {                       \
                        if (condition) {           \
                        fprintf(stderr,"ERROR CAUGHT BY ERRORHANDLER\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);     \
                        fprintf(stderr,__VA_ARGS__);  \
                        fprintf(stderr,"\n");   \
                        fflush(stderr);                      \
                        exit(EXIT_FAILURE);                                             \
                      }                                                                 \
                    } while(0)



/**
 * structure to hold sensor data
 */
typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;
} sensor_data_t;


/**
 * structure to hold sensor data in packed format
 */
typedef struct {
    sensor_id_t id __attribute__((packed));         /** < sensor id */
    sensor_value_t value __attribute__((packed));   /** < sensor value */
    sensor_ts_t ts __attribute__((packed));    /** < sensor timestamp */
} sensor_data_packed_t;


void write_to_logger(char* to_write);

#endif /* _CONFIG_H_ */
