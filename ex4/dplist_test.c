/**
 * \author Ward Smets
 */

#define _GNU_SOURCE

#include "dplist.h"
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>



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
    // Add other tests here...

    srunner_run_all(sr, CK_VERBOSE);

    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
