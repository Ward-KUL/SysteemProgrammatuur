#define _GNU_SOURCE

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/dplist.h"
#include "datamgr.h"


void setup(void) {
    // Implement pre-test setup
}

void teardown(void) {
    // Implement post-test teardown
}


typedef struct 
{
    FILE* data;
    FILE* map;
}files_t;

files_t open_files(){
    FILE* data = fopen("sensor_data","r");
    FILE* map = fopen("room_sensor.map","r");
    files_t files;
    files.data = data;
    files.map = map;
    return files;
}


START_TEST(test_basic_functionality_list){
    dplist_t* list = dpl_create(NULL,NULL,NULL);
    ck_assert(dpl_get_first_reference(list) == NULL);
    dpl_insert_at_index(list,NULL,0,false);
    ck_assert(dpl_get_first_reference(list) != NULL);
    dpl_free(&list,false);
}
END_TEST

START_TEST(test_read_file){
    files_t files = open_files();
    datamgr_parse_sensor_files(files.map,files.data);
    ck_assert(13.9 < datamgr_get_avg(15) && datamgr_get_avg(15) < 14.1);
    datamgr_free();
}
END_TEST

START_TEST(test_free_datamgr){
    files_t files = open_files();
    datamgr_free();
    datamgr_parse_sensor_files(files.map,files.data);


}
END_TEST

int main(void) {
    Suite *s1 = suite_create("LIST_EX3");
    TCase *tc1_1 = tcase_create("Core");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_checked_fixture(tc1_1, setup, teardown);
    tcase_add_test(tc1_1,test_read_file);
    tcase_add_test(tc1_1,test_basic_functionality_list);
    tcase_add_test(tc1_1,test_free_datamgr);



    srunner_run_all(sr, CK_VERBOSE);

    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}