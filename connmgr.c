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
// #include <stropts.h>


#define PORT 5678



/**
 * Structure for holding the TCP socket information
 */
struct tcpsock {
    long cookie;        /**< if the socket is bound, cookie should be equal to MAGIC_COOKIE */
    // remark: the use of magic cookies doesn't guarantee a 'bullet proof' test
    int sd;             /**< socket descriptor */
    char *ip_addr;      /**< socket IP address */
    int port;           /**< socket port number */
};



void free_tcp(void** tcp){
    free(*tcp);
}
void* copy_tcp(void* tcp){
    tcpsock_t* copy = malloc(sizeof(tcpsock_t));
    *copy = *(tcpsock_t*)tcp;
    return copy;
}

/**
 * Implements a sequential test server (only one connection at the same time)
 */

int main(void) {

    tcpsock_t *server, *client;
    sensor_data_t data;
    int bytes, result;
    int conn_counter = 0;
    dplist_t* tcp_list = dpl_create(copy_tcp,free_tcp,NULL); //NEEDS TO BE FREED STILL    

    printf("Test server is started\n");
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    dpl_insert_at_index(tcp_list,server,99,true);
    do {
        //poll for connections
        conn_counter = dpl_size(tcp_list);
        struct pollfd fds[conn_counter];
        for(int i = 0;i<conn_counter;i++){
            // add new fds to the fds to be polled
            tcpsock_t* temp = dpl_get_element_at_index(tcp_list,i);
            tcp_get_sd(temp,&fds[i].fd);
            fds[i].events = POLLIN;
        }
        int ret = poll(fds,conn_counter,TIMEOUT);
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
                        dpl_insert_at_index(tcp_list,client,99,true);
                    }
                    else{
                        //iets op de clients veranderd
                        do {
                            client = dpl_get_element_at_index(tcp_list,x);
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
                            int sd;
                            tcp_get_sd(client,&sd);
                            for(int i = 0;i<conn_counter;i++){
                                tcpsock_t* temp = (tcpsock_t*)dpl_get_element_at_index(tcp_list,i);
                                int element;
                                tcp_get_sd(temp,&element);
                                if(element == sd){
                                    tcp_close(&client);
                                    dpl_remove_at_index(tcp_list,i,false);
                                    break;
                                }
                            }
                        }
                    }
                }
                
            }
        }
    } while (1);//keep it running
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("Test server is shutting down\n");
    dpl_free(&tcp_list,true);
    return 0;
}




