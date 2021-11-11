/**
 * \author Ward Smets
 */

#define _GNU_SOURCE

#include "dplist.h"
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

typedef struct {
    int id;
    char* name;
} my_element_t;

void* element_copy(void * element);
void element_free(void ** element);
int element_compare(void * x, void * y);

void * element_copy(void * element) {
    my_element_t* copy = malloc(sizeof (my_element_t));
    char* new_name;
    asprintf(&new_name,"%s",((my_element_t*)element)->name);
    assert(copy != NULL);
    copy->id = ((my_element_t*)element)->id;
    copy->name = new_name;
    return (void *) copy;
}

void element_free(void ** element) {
    //printf("This should be the element: %c\n",*(((my_element_t*)*element))->name);
    free((((my_element_t*)*element))->name);
    free(*element);
    *element = NULL;
}

int element_compare(void * x, void * y) {
    return ((((my_element_t*)x)->id < ((my_element_t*)y)->id) ? -1 : (((my_element_t*)x)->id == ((my_element_t*)y)->id) ? 0 : 1);
}

my_element_t* make_element(int id,char name_c){
    my_element_t* element = malloc(sizeof(my_element_t));
    element->id = 1;
    char* name = malloc(sizeof(char));
    *name = name_c; 
    element->name = name;
    return element;
}

dplist_t* make_list_with_some_elements(){
    my_element_t* e1 = make_element(1,'a');
    my_element_t* e2 = make_element(2,'b');
    my_element_t* e3 = make_element(3,'c');
    my_element_t* e4 = make_element(4,'d');
    my_element_t* e5 = make_element(5,'e');
    my_element_t* e6 = make_element(6,'f');
    dplist_t* list = dpl_create(element_copy,element_free,element_compare);
    dpl_insert_at_index(list,e1,0,false);
    dpl_insert_at_index(list,e2,1,false);
    dpl_insert_at_index(list,e3,2,false);
    dpl_insert_at_index(list,e4,3,false);
    dpl_insert_at_index(list,e5,4,false);
    dpl_insert_at_index(list,e6,5,false);
    return list;
}

void setup(void) {
    // Implement pre-test setup
}

void teardown(void) {
    // Implement post-test teardown
}
START_TEST(test_ListFree)
    {
        dplist_t* list = NULL;
        dpl_free(&list,false);
        ck_assert_msg(list == NULL,"Failure: expected null");
        list = NULL;
        dpl_free(&list,true);
        ck_assert_msg(list == NULL,"Failure: expected null");

        list = dpl_create(element_copy,element_free,element_compare);
        dpl_free(&list,false);
        ck_assert_msg(list == NULL,"Failure: expected null");
        list = dpl_create(element_copy,element_free,element_compare);
        dpl_free(&list,true);
        ck_assert_msg(list == NULL,"Failure: expected null");

        my_element_t* e1 = make_element(4,'a');
        element_free((void**)&e1);

        list = dpl_create(element_copy,element_free,element_compare);
        my_element_t* element = make_element(1,'a');
        dpl_insert_at_index(list,element,0,false);
        dpl_free(&list,false);
        element_free((void**) &element);
        list = dpl_create(element_copy,element_free,element_compare);
        element = make_element(2,'b');
        dpl_insert_at_index(list,element,0,false);
        dpl_free(&list,true); 
        ck_assert_msg(list == NULL,"Failure: expected null");

        list = dpl_create(element_copy,element_free,element_compare);
        my_element_t* e2 = make_element(4,'c');
        my_element_t* e3 = make_element(5,'d');
        dpl_insert_at_index(list,e2,0,false);
        dpl_insert_at_index(list,e3,1,false);
        dpl_free(&list,false);
        element_free((void**)&e2);
        element_free((void**)&e3);
        ck_assert_msg(list == NULL,"Failure: expected null");

        list = dpl_create(element_copy,element_free,element_compare);
        my_element_t* e6 = make_element(4,'c');
        my_element_t* e7 = make_element(5,'d');
        dpl_insert_at_index(list,e6,0,false);
        dpl_insert_at_index(list,e7,1,false);
        dpl_free(&list,true);

        list = dpl_create(element_copy,element_free,element_compare);
        my_element_t* e8 = make_element(4,'c');
        my_element_t* e9 = make_element(5,'d');
        dpl_insert_at_index(list,e8,0,false);
        dpl_insert_at_index(list,e9,0,false);
        dpl_free(&list,true);

    }
END_TEST

START_TEST(test_insert){
   dplist_t* list = NULL;
   my_element_t* e = make_element(1,'a');
   ck_assert_msg(dpl_insert_at_index(list,e,0,false)==NULL,"ex null");
   ck_assert_msg(dpl_insert_at_index(list,e,-1,false)==NULL,"ex null");
   ck_assert_msg(dpl_insert_at_index(list,e,99,false)==NULL,"ex null");
   ck_assert_msg(dpl_insert_at_index(list,e,0,true)==NULL,"ex null");
   ck_assert_msg(dpl_insert_at_index(list,e,-1,true)==NULL,"ex null");
   ck_assert_msg(dpl_insert_at_index(list,e,99,true)==NULL,"ex null");
   element_free((void**)&e);
   ck_assert_msg(dpl_insert_at_index(list,NULL,0,true) == NULL,"ex null");
   dpl_free(&list,false);

    list = dpl_create(element_copy,element_free,element_compare);
    my_element_t* e1 = make_element(1,'a');
    dpl_insert_at_index(list,e1,0,false);
    ck_assert_msg(dpl_get_element_at_index(list,0)== e1,"failure");
    my_element_t* e2 = make_element(2,'b');
    dpl_insert_at_index(list,e2,-1,false);
    ck_assert_msg(dpl_get_element_at_index(list,-1)== e2,"failure");
    my_element_t* e3 = make_element(3,'c');
    dpl_insert_at_index(list,e3,0,false);
    ck_assert_msg(dpl_get_element_at_index(list,0) == e3,"F");
    ck_assert_msg(dpl_get_element_at_index(list,1) == e2,"F");
    ck_assert_msg(dpl_get_element_at_index(list,99) == e1,"F");
    my_element_t* e4 = make_element(4,'d');
    dpl_insert_at_index(list,e4,99,false);
    dpl_insert_at_index(list,NULL,4,false);
    ck_assert_msg(dpl_get_element_at_index(list,3) == e4,"F");
    ck_assert_msg(dpl_get_element_at_index(list,4) == NULL,"F");
    dpl_free(&list,true);

    //add to list with copies
    list = dpl_create(element_copy,element_free,element_compare);
    my_element_t* e6 = make_element(6,'f');
    dpl_insert_at_index(list,e6,0,true);
    ck_assert_msg(*((my_element_t*)dpl_get_element_at_index(list,0))->name == *(e6->name),"F");
    dpl_free(&list,true);
    ck_assert_msg(*e6->name = 'f',"F");
    element_free((void**)&e6);

    //geting from null list
    list = NULL;
    ck_assert_msg(dpl_get_element_at_index(list,-1) == NULL,"F");
    ck_assert_msg(dpl_get_element_at_index(list,1) == NULL,"F");
    ck_assert_msg(dpl_get_element_at_index(list,0) == NULL,"F");
    ck_assert_msg(dpl_get_element_at_index(list,99) == NULL,"F");

    //getting from empty list
    list = dpl_create(element_copy,element_free,element_compare);
    ck_assert_msg(dpl_get_element_at_index(list,-1) == NULL,"F");
    ck_assert_msg(dpl_get_element_at_index(list,1) == NULL,"F");
    ck_assert_msg(dpl_get_element_at_index(list,0) == NULL,"F");
    ck_assert_msg(dpl_get_element_at_index(list,99) == NULL,"F");
    dpl_free(&list,true);


}
END_TEST

START_TEST(test_remove){
    //null list
    dplist_t* list = NULL;
    dpl_remove_at_index(list,0,false);
    ck_assert_msg(dpl_remove_at_index(list,0,false) == NULL,"F");
    ck_assert_msg(dpl_remove_at_index(list,0,true) == NULL,"F");
    ck_assert_msg(dpl_remove_at_index(list,-1,false) == NULL,"F");
    ck_assert_msg(dpl_remove_at_index(list,-1,true) == NULL,"F");
    ck_assert_msg(dpl_remove_at_index(list,99,false) == NULL,"F");
    ck_assert_msg(dpl_remove_at_index(list,99,true) == NULL,"F");
    //empty list
    list = dpl_create(element_copy,element_free,element_compare);
    ck_assert_msg(dpl_remove_at_index(list,0,false) == list,"F");
    ck_assert_msg(dpl_remove_at_index(list,0,true) == list,"F");
    ck_assert_msg(dpl_remove_at_index(list,-1,false) == list,"F");
    ck_assert_msg(dpl_remove_at_index(list,-1,true) == list,"F");
    ck_assert_msg(dpl_remove_at_index(list,99,false) == list,"F");
    ck_assert_msg(dpl_remove_at_index(list,99,true) == list,"F");
    dpl_free(&list,true);

    //with elements
    my_element_t* e1 = make_element(1,'a');
    my_element_t* e2 = make_element(2,'b');
    my_element_t* e3 = make_element(3,'c');
    list = dpl_create(element_copy,element_free,element_compare);
    dpl_insert_at_index(list,e1,0,false);
    dpl_insert_at_index(list,e2,1,false);
    dpl_insert_at_index(list,e3,2,false);
    ck_assert_msg(dpl_get_element_at_index(list,1) == e2,"F");
    dpl_remove_at_index(list,1,true);//remove e2
    ck_assert_msg(dpl_get_element_at_index(list,1) == e3,"F");
    dpl_remove_at_index(list,-3,false);//remove e1
    ck_assert_msg(dpl_get_element_at_index(list,-4) == e3,"F");
    dpl_free(&list,true);
    element_free((void**)&e1);


   
    


}
END_TEST

START_TEST(test_size){
    dplist_t* list = NULL;
    ck_assert_msg(dpl_size(list) == -1,"F");
    list = dpl_create(element_copy,element_free,element_compare);
    ck_assert_msg(dpl_size(list) == 0,"F");
    my_element_t* e = make_element(1,'d');
    dpl_insert_at_index(list,e,0,false);
    ck_assert_msg(dpl_size(list) == 1,"F");
    dpl_insert_at_index(list,e,0,true);
    ck_assert_msg(dpl_size(list) == 2,"F");
    dpl_free(&list,true);

}
END_TEST

START_TEST(test_insertSameElement){
    dplist_t* list = dpl_create(element_copy,element_free,element_compare);
    my_element_t* e1 = make_element(1,'a');
    dpl_insert_at_index(list,e1,0,false);
    dpl_insert_at_index(list,e1,0,false);
    ck_assert_msg(dpl_size(list) == 2,"F");
    dpl_free(&list,true);

}
END_TEST

START_TEST(test_getReference){
    dplist_t* list = dpl_create(element_copy,element_free,element_compare);
    my_element_t* e1 = make_element(1,'a');
    dpl_insert_at_index(list,e1,0,false);
    dplist_node_t* ref = dpl_get_reference_at_index(list,0);
    my_element_t* e2 = (my_element_t*)(((dplist_node_t*)ref)->element);
    ck_assert_msg(ref->element == e1,"F");
    my_element_t* e3 = make_element(2,'b');
    dpl_insert_at_index(list,e3,1,false);
    ck_assert_msg(dpl_get_reference_at_index(list,0)->element ==  e1,"F");
    ck_assert_msg(dpl_get_reference_at_index(list,1)->element ==  e3,"F");
    ck_assert_msg(dpl_get_reference_at_index(list,-1)->element ==  e1,"F");
    ck_assert_msg(dpl_get_reference_at_index(list,99)->element ==  e3,"F");
    dpl_free(&list,true);
    //empty and null list
    list = NULL;
    ck_assert_msg(dpl_get_reference_at_index(list,0) ==  NULL,"F");
    ck_assert_msg(dpl_get_reference_at_index(list,1) ==  NULL,"F");
    ck_assert_msg(dpl_get_reference_at_index(list,-1) ==  NULL,"F");
    ck_assert_msg(dpl_get_reference_at_index(list,99) ==  NULL,"F");
    list = dpl_create(element_copy,element_free,element_compare);
    ck_assert_msg(dpl_get_reference_at_index(list,0) ==  NULL,"F");
    ck_assert_msg(dpl_get_reference_at_index(list,1) ==  NULL,"F");
    ck_assert_msg(dpl_get_reference_at_index(list,-1) ==  NULL,"F");
    ck_assert_msg(dpl_get_reference_at_index(list,99) ==  NULL,"F");
    dpl_free(&list,true);

}
END_TEST

START_TEST(test_getIndexOfElement){
    dplist_t* list = NULL;
    my_element_t* e = make_element(1,'a');
    ck_assert_msg(dpl_get_index_of_element(list,NULL) == -1,"F");
    ck_assert_msg(dpl_get_index_of_element(list,e) == -1,"F");
    dpl_insert_at_index(list,e,0,false);
    ck_assert_msg(dpl_get_index_of_element(list,e) == -1,"F");
    element_free((void**)&e);
    
    list = make_list_with_some_elements();
    my_element_t* e5 = make_element(1,'e');
    ck_assert_msg(dpl_get_index_of_element(list,NULL) == -1,"F");
    ck_assert_msg(dpl_get_index_of_element(list,e5) == -1,"F");
    dpl_insert_at_index(list,e5,0,false);
    ck_assert_msg(dpl_get_index_of_element(list,e5) == 0,"F");
    dpl_free(&list,true);

    list = dpl_create(element_copy,element_free,element_compare);
    my_element_t* e7 = make_element(7,7);
    ck_assert_msg(dpl_get_index_of_element(list,NULL) == -1,"F");
    ck_assert_msg(dpl_get_index_of_element(list,e) == -1,"F");
    dpl_insert_at_index(list,e7,0,false);
    ck_assert_msg(dpl_get_index_of_element(list,e) == -1,"F");
    dpl_free(&list,true);

}
END_TEST

START_TEST(test_get_first_reference){
    dplist_t* list = NULL;
    ck_assert(dpl_get_first_reference(list) == NULL);
    list = dpl_create(element_copy,element_free,element_compare);
    ck_assert_msg(dpl_get_first_reference(list) == NULL,"F");
    my_element_t* e = make_element(1,'a');
    dpl_insert_at_index(list,e,-1, false);
    ck_assert_msg((dpl_get_first_reference(list))->element == e,"F");
    dpl_free(&list,true);
    list = make_list_with_some_elements();
    my_element_t* e2 = make_element(1,'a');
    dpl_insert_at_index(list,e2,-1, false);
    ck_assert_msg(dpl_get_first_reference(list)->element == e2,"F");
    dpl_free(&list,true);
    
}
END_TEST

START_TEST(test_get_last_reference){
    dplist_t* list = NULL;
    ck_assert(dpl_get_last_reference(list) == NULL);
    list = dpl_create(element_copy,element_free,element_compare);
    ck_assert_msg(dpl_get_last_reference(list) == NULL,"F");
    my_element_t* e = make_element(1,'a');
    dpl_insert_at_index(list,e,-1, false);
    ck_assert_msg((dpl_get_last_reference(list))->element == e,"F");
    dpl_free(&list,true);
    list = make_list_with_some_elements();
    my_element_t* e2 = make_element(1,'a');
    dpl_insert_at_index(list,e2,99, false);
    ck_assert_msg(dpl_get_last_reference(list)->element == e2,"F");
    dpl_free(&list,true);
    
}
END_TEST

START_TEST(test_get_next_reference){
    dplist_t* list = NULL;
    ck_assert(dpl_get_next_reference(list,NULL) == NULL);
    list = make_list_with_some_elements();
    dplist_node_t* ref = dpl_get_reference_at_index(list,3);
    ck_assert(dpl_get_next_reference(list,ref) == dpl_get_reference_at_index(list,4));
    ck_assert(dpl_get_next_reference(list,dpl_get_reference_at_index(list,0)) == dpl_get_reference_at_index(list,1));
    ck_assert(dpl_get_next_reference(list,dpl_get_reference_at_index(list,99)) == NULL);
    dpl_remove_at_index(list,3,true);
    ck_assert(dpl_get_reference_at_index(list,2) != NULL);
    ck_assert(dpl_get_next_reference(list,dpl_get_reference_at_index(list,2)) == dpl_get_reference_at_index(list,3));
    dplist_t* list2 = dpl_create(element_copy,element_free,element_compare);
    ck_assert(dpl_get_next_reference(list2,ref)==NULL);
    dpl_insert_at_index(list2,make_element(1,'b'),0,false);
    ck_assert(dpl_get_next_reference(list2,ref)==NULL);
    ck_assert(dpl_get_next_reference(list,NULL) == NULL);
    ck_assert(dpl_get_next_reference(list,dpl_get_reference_at_index(list,99)) == NULL);
    dpl_free(&list,true);
    dpl_free(&list2,true);
}
END_TEST

START_TEST(test_get_previous_reference){
    dplist_t* list = NULL;
    ck_assert(dpl_get_previous_reference(list,NULL) == NULL);
    list = make_list_with_some_elements();
    dplist_node_t* ref = dpl_get_reference_at_index(list,3);
    ck_assert(dpl_get_previous_reference(list,ref) == dpl_get_reference_at_index(list,2));
    ck_assert(dpl_get_previous_reference(list,dpl_get_reference_at_index(list,99)) == dpl_get_reference_at_index(list,dpl_size(list)-2));
    ck_assert(dpl_get_previous_reference(list,dpl_get_reference_at_index(list,0)) == NULL);
    dpl_remove_at_index(list,3,true);
    ck_assert(dpl_get_reference_at_index(list,4) != NULL);
    ck_assert(dpl_get_previous_reference(list,dpl_get_reference_at_index(list,4)) == dpl_get_reference_at_index(list,3));
    dplist_t* list2 = dpl_create(element_copy,element_free,element_compare);
    ck_assert(dpl_get_previous_reference(list2,ref)==NULL);
    dpl_insert_at_index(list2,make_element(1,'b'),0,false);
    ck_assert(dpl_get_previous_reference(list2,ref)==NULL);
    ck_assert(dpl_get_previous_reference(list,NULL) == NULL);
    ck_assert(dpl_get_previous_reference(list,dpl_get_reference_at_index(list,0)) == NULL);
    dpl_free(&list,true);
    dpl_free(&list2,true);
}
END_TEST

START_TEST(test_get_element_at_reference){
    dplist_t* listE = NULL;
    ck_assert(dpl_get_element_at_reference(listE,NULL) == NULL);
    dplist_t* list = make_list_with_some_elements();
    ck_assert(dpl_get_element_at_reference(list,NULL) == NULL);
    ck_assert(dpl_get_element_at_reference(listE,dpl_get_reference_at_index(list,0)) == NULL);
    ck_assert(dpl_get_element_at_reference(list,dpl_get_reference_at_index(list,3)) == dpl_get_element_at_index(list,3));
    ck_assert(dpl_get_element_at_reference(list,dpl_get_reference_at_index(list,0)) == dpl_get_element_at_index(list,0));
    ck_assert(dpl_get_element_at_reference(list,dpl_get_reference_at_index(list,99)) == dpl_get_element_at_index(list,99));
    dplist_t* list2 = make_list_with_some_elements();
    ck_assert(dpl_get_element_at_reference(list,dpl_get_reference_at_index(list2,0)) == NULL);

    dpl_free(&list,true);
    dpl_free(&list2,true);
    dpl_free(&listE,true);


}
END_TEST

//START_TEST(test_nameOfYourTest)
//  Add other testcases here...
//END_TEST

int main(void) {
    Suite *s1 = suite_create("LIST_EX3");
    TCase *tc1_1 = tcase_create("Core");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_checked_fixture(tc1_1, setup, teardown);
    tcase_add_test(tc1_1, test_ListFree);
    tcase_add_test(tc1_1,test_remove);
    tcase_add_test(tc1_1,test_insert);
    tcase_add_test(tc1_1,test_size);
    //tcase_add_test(tc1_1,test_insertSameElement);
    tcase_add_test(tc1_1,test_getReference);
    tcase_add_test(tc1_1,test_getIndexOfElement);
    tcase_add_test(tc1_1,test_get_first_reference);
    tcase_add_test(tc1_1,test_get_last_reference);
    tcase_add_test(tc1_1,test_get_next_reference);
    tcase_add_test(tc1_1,test_get_previous_reference);
    tcase_add_test(tc1_1,test_get_element_at_reference);






    
    // Add other tests here...

    srunner_run_all(sr, CK_VERBOSE);

    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
