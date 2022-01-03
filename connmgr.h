/**
 * \author Ward Smets
 */

#ifndef CONNMGR_H_
#define CONNMGR_H_

#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "sbuffer.h"


#ifndef TIMEOUT
#error TIMEOUT not set
#endif

typedef struct active_connection active_connection_t;

void connmgr_listen(int port_number,sbuffer_t* buffer);

void connmgr_free();


#endif  //CONNMGR_H_