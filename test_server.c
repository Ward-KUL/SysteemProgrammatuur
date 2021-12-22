/**
 * \author Ward Smets
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "lib/tcpsock.h"
#include "lib/dplist.h"
#include <poll.h>
// #include <stropts.h>


#define PORT 5678
#define MAX_CONN 3  // state the max. number of connections the server will handle before exiting
#define TIMEOUT -1

void free_tcp(void** tcp){
    free(*tcp);
}
void* copy_tcp(void* tcp){
    printf("TRIED TO USE A FUNCTION WHICH IS NOT IMPLEMENTED\n\n\n\n");
    // int* copy  = malloc(sizeof(int));
    // *copy = *((int*)(tcp));
    // return copy;
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
    int* sd = malloc(sizeof(int)); //socket of server
    tcp_get_sd(server,sd);
    dpl_insert_at_index(tcp_list,sd,99,false);
    do {
        //poll for connections
        conn_counter = dpl_size(tcp_list);
        // printf("Amount of connections %d \n",conn_counter);
        struct pollfd fds[conn_counter];
        for(int i = 0;i<conn_counter;i++){
            // add new fds to the fds to be polled
            fds[i].fd = *((int*)dpl_get_element_at_index(tcp_list,i));
            fds[i].events = POLLIN;
        }
        int ret = poll(fds,conn_counter,10000000);
        //something has happened -> check which device
        if(ret>0){
            // printf("currently %d poll's responses\n", ret);
            for(int x = 0;x<conn_counter;x++){
                if(fds[x].revents & POLLIN){
                    fds[x].revents = 0; //clear the flag again
                    fds[x].events = 0;
                    fds[x].fd = -1;
                    if(x == 0){
                        printf("Server does something\n");
                        //er is iets op de server gebeurd(een nieuwe connectie)
                        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) exit(EXIT_FAILURE);
                        printf("Incoming client connection\n");
                        conn_counter++;
                        int* sd = malloc(sizeof(int));
                        tcp_get_sd(client,sd);
                        dpl_insert_at_index(tcp_list,sd,99,false);
                    }
                    else{
                        printf("Client does something\n");
                        //iets op de clients veranderd
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
                            printf("before %d\n",dpl_size(tcp_list));
                            int sd;
                            tcp_get_sd(client,&sd);
                            for(int i = 0;i<conn_counter;i++){
                                int element = *(int*)dpl_get_element_at_index(tcp_list,i);
                                if(element == sd){
                                    dpl_remove_at_index(tcp_list,i,true);
                                    break;
                                }
                            }
                            tcp_close(&client);
                            printf("after %d\n",dpl_size(tcp_list));
                        }
                    }
                }
                
            }
        }
    } while (1);//keep it running
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("Test server is shutting down\n");
    return 0;
}




