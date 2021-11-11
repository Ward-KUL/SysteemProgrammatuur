/**
 * \author Ward Smets
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * definition of error codes
 * */
#define DPLIST_NO_ERROR 0
#define DPLIST_MEMORY_ERROR 1 // error due to mem alloc failure
#define DPLIST_INVALID_ERROR 2 //error due to a list operation applied on a NULL list 

#ifdef DEBUG
#define DEBUG_PRINTF(...) 									                                        \
        do {											                                            \
            fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	    \
            fprintf(stderr,__VA_ARGS__);								                            \
            fflush(stderr);                                                                         \
                } while(0)
#else
#define DEBUG_PRINTF(...) (void)0
#endif


#define DPLIST_ERR_HANDLER(condition, err_code)                         \
    do {                                                                \
            if ((condition)) DEBUG_PRINTF(#condition " failed\n");      \
            assert(!(condition));                                       \
        } while(0)


/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};

dplist_node_t* create_new_node(void* element);

dplist_node_t* dpl_get_reference_in_list_if_exists(dplist_t* list, dplist_node_t* ref){
    if(list == NULL||ref == NULL) return NULL;
    dplist_node_t* node = list->head;
    while(node!=NULL){
        if(node == ref){
            return node;
        }
        node = node->next;
    }
    return NULL;
}


dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_MEMORY_ERROR);
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}


void dpl_free(dplist_t **list, bool free_element) {

    if(*list != NULL){
        dplist_node_t* node = (*list)->head;
        dplist_node_t* previous = NULL;
        while(node != NULL){
            if((free_element == true)&&(node->element!=NULL)){
                (*list)->element_free(&(node->element));//call element free
                //(*(&node))->element = NULL; //and set pointer to null so we don't want to clear the same element twice
            } 
            previous = node;
            node = node->next;
            free(previous);
        }
        free(*list);
        node = NULL;
        *list = NULL;

    }

}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {


    if(list == NULL) return NULL;
    
    if(list->head == NULL) {
        //the element given will aways be the firste element in the list
        if(insert_copy == true && element!=NULL)
            list->head = create_new_node(list->element_copy(element));
        else
            list->head = create_new_node(element);
    }
    else{
        dplist_node_t* node = NULL;
            if(insert_copy == true && element!=NULL)
                node = create_new_node(list->element_copy(element));
            else
                node = create_new_node(element);
            if(index<= 0){
                node->next = list->head;
                list->head->prev = node;
                list->head = node;
            }
            else{
                int size = dpl_size(list);
                if(index >(size -1)){
                    //add a new element at the back of the list
                    node->prev = dpl_get_reference_at_index(list,size - 1);
                    node->prev->next = node;
                }
                else{
                    node->prev = dpl_get_reference_at_index(list,index-1);
                    node->next = node->prev->next;
                    node->prev->next = node;
                    node->next->prev = node;
                }
            }
            return list;

    }

}

dplist_node_t* create_new_node(void* element){
    //printf("%d: created an element of size %ld\ns",__LINE__,sizeof(dplist_node_t));
    dplist_node_t* newNode = malloc(sizeof(dplist_node_t));
    newNode->element = element;
    newNode->next = NULL;
    newNode->prev = NULL;
    return newNode;
}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {
    if(list == NULL) return NULL;
    dplist_node_t* node;
    if(list->head == NULL) return list;
    node = list->head;
    unsigned int size = dpl_size(list);
    if(index<=0) {
        list->head = node->next;
        if(free_element == true)
            list->element_free(&(node->element));
        free(node);
        return list;
    }
    else if(index>(size-1)) index = size -1;
    //loop till at the correct node
    int i = 0;
    while(i <(index)){
        DPLIST_ERR_HANDLER(node==NULL,DPLIST_MEMORY_ERROR);
        node = node->next;
        i++;    
    }
    //arived at the correct index -1
    //make node point to the next one
    node->prev->next = node->next;
    if(free_element == true)
        list->element_free(&(node->element));
    free(node);
    return list;
    

}

int dpl_size(dplist_t *list) {

    if(list == NULL) return -1;
    dplist_node_t* node = list->head;
    int size = 0;
    while(node != NULL){
        node = node->next;
        size ++;
    }
    return size;

}

void *dpl_get_element_at_index(dplist_t *list, int index) {

    if((list == NULL )|| (list->head == NULL)) return NULL;
    if(index<=0) return list->head->element;
    int size = dpl_size(list);
    if(index >= size - 1){
        index = size - 1;
    }
    dplist_node_t* node = list->head;
    int counter = 0;
    while(counter<index){
        node = node->next;
        counter ++;
    }
    return node->element;

}

int dpl_get_index_of_element(dplist_t *list, void *element) {

    if(list == NULL || element==NULL) return -1;
    if(list->head == NULL) return -1; //list is empty
    dplist_node_t* node = list->head;
    //dplist_node_t* previousNode;
    int index = 0;
    while(node != NULL){
        if(node->element == element) return index; 
        node = node->next;
        index++;
    }
    return -1;//element not found
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {

    if(list == NULL) return NULL;
    int count;
    dplist_node_t *dummy;
    if (list->head == NULL) return NULL;
    for (dummy = list->head, count = 0; dummy->next != NULL; dummy = dummy->next, count++) {
        if (count >= index) return dummy;
    }//TODO: add your code here
    return dummy;
}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {
    //OLD CODE
    // if(list == NULL||reference == NULL) return NULL;
    // dplist_node_t* node = list->head;
    // while(node!=NULL){
    //     if(node == reference) return node->element;
    //     node = node->next;
    // }
    // //reference not found
    // return NULL;

    //NEW CODE
    reference = dpl_get_reference_in_list_if_exists(list,reference);
    if(reference == NULL) return NULL;
    else return reference->element;

}
dplist_node_t *dpl_get_first_reference(dplist_t *list){
    if(list == NULL) return NULL;
    if(list->head == NULL) return NULL;
    return list->head;
}

dplist_node_t *dpl_get_last_reference(dplist_t *list){
    if(list == NULL) return NULL;
    if(list->head == NULL) return NULL;
    //first find last node
    dplist_node_t* node = list->head;
    if(list->head->next == NULL) return list->head;
    while(node->next != NULL){
        node = node->next;
    }
    return node;
}

dplist_node_t *dpl_get_next_reference(dplist_t *list, dplist_node_t *reference){
    //OLD CODE
    // if(list == NULL|| reference ==NULL) return NULL;
    // if(list->head == NULL) return NULL;
    // dplist_node_t* node = list->head;
    // while(node!=NULL){
    //     if(node == reference) return node->next;
    //     node = node->next;
    // }
    // //refence not found
    // return NULL;

    //NEW CODE
    reference = dpl_get_reference_in_list_if_exists(list,reference);
    if(reference!=NULL) return reference->next;
    else return reference;
}

dplist_node_t *dpl_get_previous_reference(dplist_t *list, dplist_node_t *reference){
    // if(list == NULL|| reference ==NULL) return NULL;
    // if(list->head == NULL) return NULL;
    // dplist_node_t* node = list->head;
    // while(node!=NULL){
    //     if(node == reference) return node->prev;
    //     node = node->next;
    // }
    // //refence not found
    // return NULL;
    reference = dpl_get_reference_in_list_if_exists(list,reference);
    if(reference!=NULL) return reference->prev;
    else return reference;
}

dplist_node_t *dpl_get_reference_of_element(dplist_t *list, void *element){
    if(list == NULL||element==NULL) return NULL;
    if(list->head == NULL) return NULL;
    dplist_node_t* node = list->head;
    while(node!=NULL){
        if(node->element == element) return node;
        node = node->next;
    }
    //element not found
    return NULL;
}

int dpl_get_index_of_reference(dplist_t *list, dplist_node_t *reference){
    if(list==NULL||reference ==NULL) return -1;
    dplist_node_t* node = list->head;
    int counter = 0;
    while(node!=NULL){
        if(node == reference) return counter;
        node = node->next;
        counter ++; 
    }
    //refernce not present
    return -1;
}

dplist_t *dpl_insert_at_reference(dplist_t *list, void *element, dplist_node_t *reference, bool insert_copy){
    //OLD CODE
    // if(list == NULL||reference == NULL) return NULL;
    // dplist_node_t* node = list->head;
    // while(node != NULL){
    //     if(node == reference){
    //         if(node->element != NULL)
    //             list->element_free(&(node->element));
    //         if(insert_copy == true && element!=NULL){
    //             node->element = list->element_copy(element);
    //             return list;
    //         }
    //         else   {
    //             node->element = element;
    //             return list;
    //         }
            
    //     } 
    //     node = node->next;
    // }
    // return NULL;
    
    //NEW CODE
    if(list ==NULL || reference == NULL) return NULL;
    reference = dpl_get_reference_in_list_if_exists(list,reference);
    if(reference == NULL) return list;
    if(reference->element != NULL)
                list->element_free(&(reference->element));
    if(insert_copy == true && element!=NULL){
        reference->element = list->element_copy(element);
            return list;
        }
    else   {
        reference->element = element;
        return list;
    }
}

dplist_t *dpl_remove_at_reference(dplist_t *list, dplist_node_t *reference, bool free_element){
    if(list == NULL||reference ==NULL||list->head == NULL) return NULL;
    reference = dpl_get_reference_in_list_if_exists(list,reference);
    if(reference == NULL) return NULL;
    if(list->head == reference){//reference is the head of the list
        list->head = reference->next;
    }
    else{
        if(reference->next != NULL) reference->next->prev = reference->prev;
        if(reference->prev != NULL) reference->prev->next = reference->next;
    }
    if(reference->element != NULL && free_element){
        list->element_free(&(reference->element));
    }
    free(reference);
    return list;
}

dplist_t *dpl_remove_element(dplist_t *list, void *element, bool free_element){
    if(list == NULL) return NULL;
    if(list->head == NULL) return NULL;
    dplist_node_t* ref = dpl_get_reference_of_element(list,element);
    if(ref == NULL) return list; //the element was not present in the list
    dpl_remove_at_reference(list,ref,free_element);
}

dplist_t *dpl_insert_sorted(dplist_t *list, void *element, bool insert_copy){

    //first find the index than insert at index
    if(list == NULL||element == NULL) return NULL;
    dplist_node_t* node = list->head;
    int counter = 0;
    while(node!=NULL && (list->element_compare(node->element,element)<0)){
        node = node->next;
        counter ++;
    }
    dpl_insert_at_index(list,element,counter,insert_copy);
    return list;


    // if(list == NULL||element == NULL) return NULL;
    // if(element != NULL && insert_copy==true) element = list->element_copy(element);
    // dplist_node_t* new = create_new_node(element);
    // if(list->head == NULL){
    //     //element is to be the first element in the list
    //     list->head = new;
    //     return list;
    // }
    // else{
    //     if((list->element_compare(element,(list->head)->element)) < 0 ){
    //         //element is smaller than the head
    //         new->next = list->head;
    //         list->head = new;
    //         return list;
    //     }
    //     else{
    //         dplist_node_t* node = list->head;
    //         dplist_node_t* lastNode = NULL;
    //         while(node != NULL){
    //             if((list->element_compare(element,node->element)) >=0 ){
    //                 //we have gotten a bigger element so we need to slide it behind this one
    //                 new->prev = node;
    //                 new->next = node->next;
    //                 if(node->next != NULL){ //check wether the element to be added is the last element
    //                     node->next = new;
    //                     new->next->prev = new;
    //                 }             
    //                 return list;
    //             }
    //             lastNode = node;
    //             node = node->next;
    //         }
    //         //the node needs to be added at the end
    //         new->prev = lastNode;
    //         lastNode->next = new;
    //         return list;
    //     }
    // }
}









