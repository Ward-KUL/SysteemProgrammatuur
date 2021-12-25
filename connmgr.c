/**
 * \author Ward Smets
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "connmgr.h"
#include "lib/tcpsock.h"
#include "lib/dplist.h"
#include <poll.h>
#include <stdbool.h>
// #include <stropts.h>





struct active_connection{
    tcpsock_t* socket;//contains the tcp socket 
    time_t ts;//contains the last timestamp
};

/**
 * Structure for holding the TCP socket information
 */
struct tcpsock{
    long cookie;        /**< if the socket is bound, cookie should be equal to MAGIC_COOKIE */
    // remark: the use of magic cookies doesn't guarantee a 'bullet proof' test
    int sd;             /**< socket descriptor */
    char *ip_addr;      /**< socket IP address */
    int port;           /**< socket port number */
};



void free_tcp(void** tcp){
    active_connection_t* conn = (active_connection_t*)(*tcp);
    //free(conn->socket); the socket is freeed automatically
    free(conn);
}


active_connection_t* get_conn(tcpsock_t* sock){
    active_connection_t* conn = malloc(sizeof(active_connection_t));
    conn->socket = sock;
    time(&(conn->ts));
    return conn;
}

/**
 * Implements a sequential test server (only one connection at the same time)
 */

//global variables
bool server_running = false;

void connmgr_free(){
    server_running = false;
    printf("Server closed externally\n");
}

void connmgr_listen(int port_number){
    server_running = true;

    tcpsock_t *server, *client;
    sensor_data_t data;
    int bytes, result;
    int conn_counter = 0;
    dplist_t* tcp_list = dpl_create(NULL,free_tcp,NULL); 

    FILE* file = fopen("sensor_data_recv","w");  

    printf("Test server has started\n");
    if (tcp_passive_open(&server, port_number) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    active_connection_t* server_conn = get_conn(server);
    dpl_insert_at_index(tcp_list,server_conn,99,false);
    do {
        //poll for connections
        conn_counter = dpl_size(tcp_list);
        struct pollfd fds[conn_counter];
        for(int i = 0;i<conn_counter;i++){
            // add new fds to the fds to be polled
            active_connection_t* temp = dpl_get_element_at_index(tcp_list,i);
            tcp_get_sd(temp->socket,&fds[i].fd);
            fds[i].events = POLLIN;
        }
        int ret = poll(fds,conn_counter,TIMEOUT*1000);
        //check for timeouts before continuing
        for(int i = 1; i<conn_counter;i++){ //and skip the server
            active_connection_t* conn = dpl_get_element_at_index(tcp_list,i);
            time_t timer;
            time(&timer);
            if((timer - conn->ts) > TIMEOUT){
                printf("Connection timed out \n");
                //the connection has timed out
                tcp_close(&(conn->socket));
                dpl_remove_at_index(tcp_list,i,true);
                //remove the result from the polling as well
                fds[i].revents = 0;
                fds[i].fd = -1;
                fds[i].events = 0;
            }
        }
        //something has happened -> check which device
        if(ret>0){
            for(int x = 0;x<conn_counter;x++){
                if(fds[x].revents & POLLIN){
                    fds[x].revents = 0; //clear the flag again
                    fds[x].events = 0;
                    fds[x].fd = -1;
                    if(x == 0){
                        //er is iets op de server gebeurd(een nieuwe connectie)
                        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) exit(EXIT_FAILURE);
                        printf("Incoming client connection\n");
                        dpl_insert_at_index(tcp_list,get_conn(client),99,false);
                    }
                    else{
                        //iets op de clients veranderd
                        active_connection_t* client_conn = dpl_get_element_at_index(tcp_list,x);
                        do {
                            client = client_conn->socket;
                            time(&(client_conn->ts));
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
                                fprintf(file,"sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
                                    (long int) data.ts);
                            }
                            //received a complete package from a sensor -> let's listen for other sensors
                            break;
                        } while (result == TCP_NO_ERROR);
                        if(result != TCP_NO_ERROR){
                            //the connection should be broken off
                            if (result == TCP_CONNECTION_CLOSED){
                                printf("Peer has closed connection\n");
                            }
                            else{
                                printf("Error occured on connection to peer\n");
                            }
                            //remove client from the list of active clients
                            tcp_close(&(client_conn->socket));
                            dpl_remove_element(tcp_list,client_conn,true);
                        }
                    }
                }
                
            }
        }
        else if(ret == 0){
            printf("Server timed out\n");
            break;
        }
    } while (server_running);//keep it running
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("Test server is shutting down\n");
    dpl_free(&tcp_list,true);
    fclose(file);
}




