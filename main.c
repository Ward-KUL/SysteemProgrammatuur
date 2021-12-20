#define _GNU_SOURCE

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include "sensor_db.h"
#include "config.h"

int print_row(void* arg1,int num_columns, char ** column_value, char** column_name){
    int i;
    for(i =1;i< num_columns; i++){
        printf("[%s=%s]\t",column_name[i],column_value[i] ? column_value[i] : NULL);
    }
    printf("\n");
    return 0;
}

void setup(void) {
    // Implement pre-test setup
}

void teardown(void) {
    // Implement post-test teardown
}

START_TEST(test_basic_db){
    printf("%s\n", sqlite3_libversion());
    ck_assert(strcmp(sqlite3_libversion(),"") != 0);
    DBCONN* conn = init_connection(1);
    disconnect(conn);
}
END_TEST

START_TEST(test_insert){
    FILE* file = fopen("sensor_data","r");
    DBCONN* conn = init_connection(1);
    insert_sensor_from_file(conn,file);
    fclose(file);
    find_sensor_all(conn,print_row);
    find_sensor_by_value(conn,15,print_row);
    find_sensor_by_timestamp(conn,1636982316,print_row);
    find_sensor_after_timestamp(conn,1636982316,print_row);
    disconnect(conn);
}
END_TEST



int main(void) {
    Suite *s1 = suite_create("LIST_EX3");
    TCase *tc1_1 = tcase_create("Core");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_checked_fixture(tc1_1, setup, teardown);

    //change timeout
   tcase_set_timeout(tc1_1,15);

    tcase_add_test(tc1_1,test_basic_db);
   tcase_add_test(tc1_1,test_insert);

   




    srunner_run_all(sr, CK_VERBOSE);

    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}