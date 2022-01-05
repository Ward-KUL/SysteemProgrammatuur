/**
 * \author Ward Smets
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "connmgr.h"
#include "lib/tcpsock.h"
#include "lib/dplist.h"
#include <poll.h>
#include <stdbool.h>
#include "sbuffer.h"
#include <unistd.h>

/**
 * container object that contains info about a tcp connection
 */
struct active_connection{
    tcpsock_t** socket;//contains the tcp socket 
    time_t ts;//contains the last timestamp
    sensor_id_t id;//contains the sensor id
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
    //the socket is freed automatically
    free(conn);
}


active_connection_t* get_conn(tcpsock_t* sock){
    active_connection_t* conn = malloc(sizeof(active_connection_t));
    conn->socket = &sock;
    time(&(conn->ts));
    conn->id = 0;
    return conn;
}


//global variables
bool server_running = false;


void connmgr_free(){
    server_running = false;
    DEBUG_PRINTF("Server closed externally\n");
}

dplist_t* close_connection(active_connection_t* conn,dplist_t* tcp_list){
    int index = dpl_get_index_of_element(tcp_list,conn);
    if(index != -1){
        // tcp_close((conn->socket));
        dpl_remove_at_index(tcp_list,index,true);
    }
    return tcp_list;
}

void tcp_error_received(active_connection_t* conn){
    char* buffer;
    asprintf(&buffer,"Error while receiving info from sensor with id: %d",conn->id);
    write_to_logger(buffer);
    free(buffer);
}

void check_if_first_connection(active_connection_t* client_conn,int id){
    if(client_conn->id == 0){
        client_conn->id = id;
        char* buffer;
        asprintf(&buffer,"A sensor with id: %d has opened a new connection",id);
        write_to_logger(buffer);
        free(buffer);
    }
}

int receive_via_tcp(active_connection_t* client_conn,sbuffer_t* buffer){
    int bytes,result;
    tcpsock_t* client = *(client_conn->socket);
    sensor_data_t data;
    // read sensor ID
    bytes = sizeof(data.id);
    result = tcp_receive(client, (void *) &data.id, &bytes);
    if(result != TCP_NO_ERROR){
        tcp_error_received(client_conn);
        return result;
    }
    // read temperature
    bytes = sizeof(data.value);
    result = tcp_receive(client, (void *) &data.value, &bytes);
    if(result!=TCP_NO_ERROR){
        tcp_error_received(client_conn);
        return result;
    }
    // read timestamp
    bytes = sizeof(data.ts);
    result = tcp_receive(client, (void *) &data.ts, &bytes);
    if(result != TCP_NO_ERROR){
        tcp_error_received(client_conn);
        return result;
    }
    check_if_first_connection(client_conn,data.id);
    ERROR_HANDLER(sbuffer_insert(buffer,&data)!=SBUFFER_SUCCESS,"Failed to insert data in the buffer");
    return TCP_NO_ERROR;
}

void receive_from_client(active_connection_t* client_conn,dplist_t* tcp_list,sbuffer_t* buffer){
    time(&(client_conn->ts));
    int result = receive_via_tcp(client_conn,buffer);
    if(result != TCP_NO_ERROR){
        //the connection is no longer valid
        if (result == TCP_CONNECTION_CLOSED){
            char* buffer;
            asprintf(&buffer,"The sensor node with id: %d has closed the connection",client_conn->id);
            write_to_logger(buffer);
            free(buffer);
        }
        else{
            char* buffer;
            asprintf(&buffer,"An error occured on sensor node with id: %d",client_conn->id);
            write_to_logger(buffer);
            free(buffer);
        }
        //remove client from the list of active clients
        close_connection(client_conn,tcp_list);
    }
}

void connmgr_listen(int port_number,sbuffer_t* buffer){
    server_running = true;

    tcpsock_t *server, *client;
    int conn_counter = 0;
    dplist_t* tcp_list = dpl_create(NULL,free_tcp,NULL); 
    write_to_logger("Test server has started");
    ERROR_HANDLER(tcp_passive_open(&server, port_number) != TCP_NO_ERROR,"Couldn't open the tcp_listener on the requested port");
    active_connection_t* server_conn = get_conn(server);
    dpl_insert_at_index(tcp_list,server_conn,99,false);
    do {
        //create the pollfd with the descriptors of the to poll i/o connections
        conn_counter = dpl_size(tcp_list);
        struct pollfd fds[conn_counter];
        for(int i = 0;i<conn_counter;i++){
            // add new fds to the fds to be polled
            active_connection_t* temp = dpl_get_element_at_index(tcp_list,i);
            tcp_get_sd(*(temp->socket),&fds[i].fd);
            fds[i].events = POLLIN;
        }
        int ret = poll(fds,conn_counter,TIMEOUT*1000);
        //remove the timed out connections
        for(int i = 1; i<conn_counter;i++){ //and skip the server
            active_connection_t* conn = dpl_get_element_at_index(tcp_list,i);
            time_t timer;
            time(&timer);
            if((timer - conn->ts) > TIMEOUT){
                char* buffer;
                asprintf(&buffer,"The sensor node with id: %d has timed out",conn->id);
                write_to_logger(buffer);
                free(buffer);
                //the connection has timed out
                tcp_list = close_connection(conn,tcp_list);
                //remove the result list of connections to poll
                fds[i].revents = 0;
                fds[i].fd = -1;
                fds[i].events = 0;
            }
        }
        //something has happened -> check which connection
        if(ret>0){
            for(int x = 0;x<conn_counter;x++){
                if(fds[x].revents & POLLIN){
                    fds[x].revents = 0; //clear the flag again
                    fds[x].events = 0;
                    fds[x].fd = -1;
                    if(x == 0){
                        //new connection
                        ERROR_HANDLER(tcp_wait_for_connection(server, &client) != TCP_NO_ERROR,"Failed to create a new connection");
                        dpl_insert_at_index(tcp_list,get_conn(client),99,false);
                    }
                    else{
                        //receive from client
                        active_connection_t* client_conn = dpl_get_element_at_index(tcp_list,x);
                        receive_from_client(client_conn,tcp_list,buffer);
                    }
                }
            }
        }
        else if(ret == 0){
            write_to_logger("Server timed out");
            break;
        }
        yield_cpu();
    } while (server_running);//keep tcp_listener running while the flag is high
    ERROR_HANDLER(tcp_close(&server) != TCP_NO_ERROR,"Failed to close server tcp connection");
    write_to_logger("Test server is shutting down");
    dpl_free(&tcp_list,true);
    sbuffer_done_writing(buffer);
}




