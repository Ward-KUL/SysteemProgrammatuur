/**
 * \author Ward Smets
 */

#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"
#include <stdbool.h>
#include <pthread.h>

/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;         /**< a structure containing the data */
    bool has_been_read;         /** boolean that will be set to true once the data has been written*/
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */
    pthread_mutex_t lock;   
};

int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    pthread_mutex_init(&((*buffer)->lock),NULL);
    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    sbuffer_node_t *dummy;
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }
    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_read_and_remove(sbuffer_t *buffer, sensor_data_t *data,sbuffer_node_t** node) {
    //het idee: ge leest de node en geeft de node dat ge gelezen hebt terug,
    //als ge niet moet verwijderen, geen probleem ga gewoon voort
    //als ge wel moet verwijderen dan zijt ge de laatste, de vorige node kunt ge dan op 
    //de head laten wijzen en kunt ge altijd de head verwijderen.
    //dan zit ge wel met 1 node achter altijd(een teveel in geheugen)
    //maar dat maakt wel op dan dat ge altijd door heel de buffer moet zitten loopen 
    //tot ge komt op de node dat ge wilt lezen
    if (buffer == NULL) return SBUFFER_FAILURE;
    if (buffer->head == NULL) return SBUFFER_NO_DATA;
    if(*node == NULL){
        //first time the buffer is read
        printf("first time reading\n");
        *node = buffer->head;
        *data = (*node)->data;
        if((*node)->has_been_read == false){
            (*node)->has_been_read = true;
        }
        return SBUFFER_SUCCESS;
    }
    else{
        //read next node
        if((*node)->next == NULL){
            return SBUFFER_NO_DATA;
        }
        pthread_mutex_lock(&(buffer->lock));
        if((*node)->next->has_been_read == false){
            pthread_mutex_unlock(&(buffer->lock));
            *node = (*node)->next;
            *data = (*node)->data;
            return SBUFFER_SUCCESS;
        }
        else{
            //second thread needs to read and remove data
            (*node)->next->has_been_read = true;
            sbuffer_node_t* temp = (*node)->next;
            free(*node);
            *node = temp;
            buffer->head = (*node)->next;//zou mss ook gewoon al node->next kunnen zijn
            *data = (*node)->data;
            pthread_mutex_unlock((&buffer->lock));
            return SBUFFER_SUCCESS;
        }

    }
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) return SBUFFER_FAILURE;
    dummy->data = *data;
    dummy->next = NULL;
    dummy->has_been_read = false;
    if (buffer->tail == NULL) // buffer empty (buffer->head should also be NULL
    {
        buffer->head = buffer->tail = dummy;
    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }
    return SBUFFER_SUCCESS;
}
