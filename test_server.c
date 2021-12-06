/**
 * \author Ward Smets
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "lib/tcpsock.h"
#include <sys/types.h>
#include <unistd.h>
#include "lib/dplist.h"
#include <poll.h>

#define PORT 5678
#define MAX_CONN 3  // state the max. number of connections the server will handle before exiting
#define TIMEOUT 2000


typedef struct{
    time_t last_responds;
    tcpsock_t* client;
}tcpconn_t;

/**
 * Implements a sequential test server (only one connection at the same time)
 */
void start_connection_manager(dplist_t* list);
void new_connection_started(tcpsock_t* server,tcpsock_t* client,dplist_t* list);
void start_timeout_listener(dplist_t* list);

void free_client(void** element){
    printf("FREE client not yet iplemented\n");
    //free((tcpsock_t*)*element);
}


int main(void) {
    dplist_t* list = dpl_create(NULL,free_client,NULL);
    pid_t childpid;
    childpid = fork();
    if(childpid == 0){
        //timeout checker
        start_timeout_listener(list);
    }
    else{
        //incoming connection listener
        start_connection_manager(list);
    }
    return 0;
}

void close_all_connections(dplist_t* list){
    for(int i = 0;i<dpl_size(list);i++){
        tcpsock_t* client = dpl_get_element_at_index(list,i);
        int result = tcp_close(&client);
        if(result != TCP_NO_ERROR) printf("Failed to close connection\n");
    }
}

void start_timeout_listener(dplist_t* list){
    while(true){
        printf("Will check for possible timeouts, amount of connections is %d\n",dpl_size(list));
        for(int i = 0;i<dpl_size(list);i++){
            tcpconn_t* conn = dpl_get_element_at_index(list,i);
            time_t time = time;
            if((conn->last_responds - time)>TIMEOUT){
                printf("Connection timed out i try to close\n");
                tcp_close(&(conn->client));
                dpl_remove_at_index(list,i,true);
            }
        }
        sleep(1);
    }
}

void start_connection_manager(dplist_t* list){
    tcpsock_t *server, *client;
    pid_t childpid;

    printf("Test server is started\n");
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    do {
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) exit(EXIT_FAILURE);
        childpid = fork();
        if(childpid == 0){
            //child process
            new_connection_started(server,client,list);
        }
        //else, the parent just keeps listening
        tcpconn_t* conn = malloc(sizeof(tcpconn_t));
        conn->client = client;
        (conn->last_responds) = (time_t)time;
        dpl_insert_at_index(list,conn,99,false);
        printf("Currently we have %d TCP_CONNECTIONS open\n",dpl_size(list));
    
    } while (1);
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("Test server is shutting down\n");
    // close_all_connections(list);
    dpl_free(&list,false);
}

void new_connection_started(tcpsock_t* server,tcpsock_t* client,dplist_t* list){
    printf("Incoming client connection\n");
    // dpl_insert_at_index(list,client,99,false);
    // printf("Currently we have %p %d TCP_CONNECTIONS open\n",conn_counter,*conn_counter);
    // *conn_counter = *conn_counter +1;
    // printf("Currently we have %p %d TCP_CONNECTIONS open\n",conn_counter,*conn_counter);

    sensor_data_t data;
    int bytes, result;
    do {
        // read sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *) &data.id, &bytes);
        // read temperature
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *) &data.value, &bytes);
        // read timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *) &data.ts, &bytes);
        if ((result == TCP_NO_ERROR) && bytes) {
            printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
                (long int) data.ts);
        }
    } while (result == TCP_NO_ERROR);
    if (result == TCP_CONNECTION_CLOSED)
        printf("Peer has closed connection\n");
    else
        printf("Error occured on connection to peer\n");
    // *conn_counter = *conn_counter -1;
    tcp_close(&client);
    exit(0);
}




