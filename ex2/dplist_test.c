/**
 * \author Ward Smets
 */

#include "dplist.h"
#include <check.h>
#include <stdlib.h>
#include <stdio.h>


void setup(void) {
    // Implement pre-test setup
    printf("%s\n","setup");
    dpl_create();
}

void teardown(void) {
    // Implement post-test teardown
    printf("teardown\n");
    //dpl_free();
}

void populate_with_numbers(dplist_t* list){
    for( char x = 0; x<10;x++){
        char *temp = malloc(sizeof(char));
        *temp = x;
        dpl_insert_at_index(list,temp,x);
    }
}
void dpl_print_list(dplist_t *list);


START_TEST(test_adding_elements)
{
    printf("TEST : Adding elements\n");
    dplist_t *list = NULL;
    ck_assert_msg(dpl_get_element_at_index(list,1) ==  NULL,"Failure: expected NULL");

    list = dpl_create();
    populate_with_numbers(list);
    ck_assert_msg(*dpl_get_element_at_index(list,3) == 3,"Failure: expected element to be 3");
    //printf("The element we got was %d\n",dpl_get_element_at_index(list,3));
    ck_assert_msg(*dpl_get_element_at_index(list,20) == 9,"Failure:expected to receive the biggest element");
    ck_assert_msg(*dpl_get_element_at_index(list,-1) == 0,"Failure: expected to receive first element, since index given was negative");
    
    


}
END_TEST
START_TEST(test_getting_element_with_reference){
    printf("TEST : Getting elements with their reference\n");
    dplist_t* list = dpl_create();  
    populate_with_numbers(list);
    char p = 3;
    ck_assert_msg(dpl_get_index_of_element(list,&p)==3,"Failure: expected to be 3");
    dplist_node_t *node = dpl_get_reference_at_index(list,3);
    //ck_assert_msg(node->element == 3,"Failure: expected to be 3");
    ck_assert_msg(dpl_size(list)==10,"Failure: expected a size of 10");
    dpl_print_list(list);
    dplist_t* list2 = NULL;
    ck_assert_msg(dpl_size(list2)==-1,"Failure: expected size to be -1 since list was null");
}
END_TEST

START_TEST(test_removing_elements){
    printf("TEST : Removing elements\n");
    //check for removing elements from the list
    dplist_t *list = dpl_create();
    populate_with_numbers(list);
    dpl_print_list(list);
    dpl_remove_at_index(list,3);
    dpl_print_list(list);
    ck_assert_msg(*dpl_get_element_at_index(list,3) == 4, "Failure: excpected to be 4");
    dpl_remove_at_index(list,-4);
    dpl_print_list(list);   
    ck_assert_msg(*dpl_get_element_at_index(list,0) == 1,"Failure: expecte to be 1");
    
    dpl_remove_at_index(list,300);
    ck_assert_msg(*dpl_get_element_at_index(list,300)==8,"Failure: expected to be 8");

    //check if list is empty after freeing it
    dpl_free(&list);
    //ck_assert_msg(dpl_size(list)==-1,"Failure: the size should be -1 since it should be cleared");  
    printf("List should be empty:  ");
    dpl_print_list(list);
    ck_assert_msg(list == NULL,"Failure: excpected list to be NULL");

}
END_TEST


START_TEST(test_ListInsertAtIndexListNULL)
    {
        printf("TEST : List insert at index list null\n");
        // Test inserting at index -1
        char p = 'A';
        dplist_t *result = dpl_insert_at_index(NULL, &p, -1);
        ck_assert_msg(result == NULL, "Failure: expected list to be NULL");
        // TODO : Test inserting at index 0
        ck_assert_msg(dpl_insert_at_index(NULL,&p,0)==NULL,"Failure: want NULL");

        // TODO : Test inserting at index 99
        ck_assert_msg(dpl_insert_at_index(NULL,&p,99)==NULL,"Failure: want NULL");
    }
END_TEST

START_TEST(test_ListInsertAtIndexListEmpty)
{
    printf("TEST : List insert at index list empty\n");
    // Test inserting at index -1
    dplist_t *list = dpl_create();
    char p = 'A';
    dplist_t *result = dpl_insert_at_index(list, &p, -1);
    ck_assert_msg(dpl_size(result) == 1, "Failure: expected list to have size of 1, got a size of %d",
                                         dpl_size(result));
    dpl_free(&list);
    // TODO : Test inserting at index 0
    list = dpl_create();
    result = dpl_insert_at_index(list, &p, 0);
    ck_assert_msg(dpl_size(result) == 1, "Failure: expected list to have size of 1, got a size of %d",
                                         dpl_size(result));
    dpl_free(&list);

    // TODO : Test inserting at index 99
    list = dpl_create();
    result = dpl_insert_at_index(list, &p, 99);
    ck_assert_msg(dpl_size(result) == 1, "Failure: expected list to have size of 1, got a size of %d",
                                         dpl_size(result));
    dpl_free(&list);
}
END_TEST

//START_TEST(test_nameOfYourTest)
//  Add other testcases here...
//END_TEST

int main(void) {
    Suite *s1 = suite_create("LIST_EX1");
    TCase *tc1_1 = tcase_create("Core");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_checked_fixture(tc1_1, setup, teardown);
    tcase_add_test(tc1_1, test_ListInsertAtIndexListNULL);
    tcase_add_test(tc1_1, test_ListInsertAtIndexListEmpty);
    // Add other tests here...
    tcase_add_test(tc1_1,test_adding_elements);
    tcase_add_test(tc1_1,test_getting_element_with_reference);
    tcase_add_test(tc1_1,test_removing_elements);

    srunner_run_all(sr, CK_VERBOSE);

    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}



