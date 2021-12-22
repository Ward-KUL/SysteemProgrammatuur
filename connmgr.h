/**
 * \author Ward Smets
 */

#ifndef CONNMGR_H_
#define CONNMGR_H_

#include <stdlib.h>
#include <stdio.h>
#include "config.h"


#ifndef TIMEOUT
#error TIMEOUT not set
#endif

/*
 * Use ERROR_HANDLER() for handling memory allocation problems, invalid sensor IDs, non-existing files, etc.
 */
#define ERROR_HANDLER(condition, ...)    do {                       \
                      if (condition) {                              \
                        printf("\nError: in %s - function %s at line %d: %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
                        exit(EXIT_FAILURE);                         \
                      }                                             \
                    } while(0)


typedef struct active_connection active_connection_t;



#endif  //CONNMGR_H_