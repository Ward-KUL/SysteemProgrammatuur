#define _GNU_SOURCE

#include "sensor_db.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


int log_count = 0;//global variable for log_count;
FILE* fifo_descr_wr;
char* close_fifo_code = "Close fifo code: 1@3k39d";
pid_t childPid;

typedef struct {
    int result_code;
    char* fifo_path;
}pipe_setup_container_t;

void write_to_logger(char* to_write){
    char *send_buf;
    asprintf(&send_buf,"%s\n",to_write);
    if(fputs(send_buf,fifo_descr_wr) == EOF){
        printf("Failed to write to fifo\n");
    }
    free(send_buf);
}

int execute_sql_stmt(DBCONN* db, char* statement,int callback, char smt){
    char* err_msg = 0;
    int rc = sqlite3_exec(db,statement,0,0,&err_msg);

    if(rc != SQLITE_OK){
        // printf("Failed to fetch data: %s \n",err_msg);
        char* msg;
        asprintf(&msg,"Failed to fetch data: %s",err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        free(msg);
        exit(EXIT_FAILURE);
    }
    return rc;
}

int check_for_SQLOK(int rc,DBCONN* conn,char interrupt_on_error){
    if(rc != SQLITE_OK){
        char* msg;
        asprintf(&msg,"Problem loading the data:%s",sqlite3_errmsg(conn));
        write_to_logger(msg);
        free(msg);
        if(interrupt_on_error == 1)
            exit(EXIT_FAILURE);
    }
    return rc;
}

int get_result_of_querry(DBCONN* conn,char* querry,callback_t f){
    char* err_msg;
    int rc = sqlite3_exec(conn,querry,f,0,&err_msg);
    return rc;
}

void start_logger(char* fifo_path){
    int MAX_BUFFER_SIZE = 150;
    char receive_buffer[MAX_BUFFER_SIZE];//set in header file with define
    char* str_result;
    time_t time_v;
    FILE* gateway = fopen("gateway.log","w");
    FILE* fifo_descr = fopen(fifo_path,"r");
    do{
        str_result = fgets(receive_buffer,MAX_BUFFER_SIZE,fifo_descr);
        if(str_result != NULL){
            //received something
            str_result[strcspn(str_result, "\n")] = 0;//haal de newline character van str_result
            time(&time_v);//set timer variable to current time
            fprintf(gateway,"<%d> <%ld> <%s>\n",log_count,time_v,str_result);
            log_count++;
        }
    }
    while(strcmp(str_result,close_fifo_code) != 0);
    printf("logger closes\n");
    //done receiving, close everything
    if(fclose(fifo_descr) != 0){
        printf("Logger couldn't close fifo\n");
    }
    fclose(gateway);
    exit(50);
}

pipe_setup_container_t setup_pipe(){
    pipe_setup_container_t cont;
    cont.fifo_path = "fifo_path";

    if(mkfifo(cont.fifo_path,0666) != 0){
        if(errno != EEXIST){
            printf("Failed to create fifo and it doesn't exist already either\n");
            exit(EXIT_FAILURE);
        }
    }

    childPid = fork();
    if(childPid == 0){
        //child process, start logger
        start_logger(cont.fifo_path);
        cont.result_code = 0;
    }
    else {
        cont.result_code = 1;
        //setup fifo to write to
        fifo_descr_wr = fopen(cont.fifo_path,"w");
        if(fifo_descr_wr == NULL){
            printf("Failed to open the fifo to write data to\n");
            exit(EXIT_FAILURE);
        }
    }
    return cont;
}



DBCONN *init_connection(char clear_up_flag){

    pipe_setup_container_t pipe_cont = setup_pipe();
    if(pipe_cont.result_code == 0) return NULL;//child process which will return once it is done reading

    
    DBCONN *db = NULL;

    
    int rc = sqlite3_open(TO_STRING(DB_NAME),&db);

    if(rc != SQLITE_OK){
        write_to_logger("Couldn't open the database");
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    else{
        char* str = "Database opened succesfully";
        write_to_logger(str);
    }

    if(clear_up_flag == 1){
        char* clear_tbl_stmt;
        asprintf(&clear_tbl_stmt,"%s %s %s","DROP TABLE IF EXISTS",TO_STRING(TABLE_NAME),";");
        execute_sql_stmt(db,clear_tbl_stmt,0,0);
        free(clear_tbl_stmt);
        write_to_logger("Cleared the table if it would have exists");

    }
    
    char* sql_stmt ;
    asprintf(&sql_stmt,"%s %s %s","CREATE TABLE IF NOT EXISTS",TO_STRING(TABLE_NAME),"(id INTEGER PRIMARY KEY AUTOINCREMENT, sensor_id	INTEGER NOT NULL, sensor_value NUMERIC NOT NULL, timestamp INTEGER NOT NULL);");
    execute_sql_stmt(db,sql_stmt,0,0);
    free(sql_stmt);
    write_to_logger("Created new table if didn't exist yet");
    
    return db;
}


void disconnect(DBCONN *conn){
    write_to_logger(close_fifo_code);
    sqlite3_close(conn);
    fclose(fifo_descr_wr);
    // int exit_code;
    // wait(&exit_code);
    return;
}

int insert_sensor(DBCONN *conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts){
    char* querry;
    asprintf(&querry,"INSERT INTO %s %s", TO_STRING(TABLE_NAME),"(sensor_id,sensor_value,timestamp) VALUES(?,?,?)");
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(conn,querry,-1,&stmt,0);
    if(rc != SQLITE_OK){
        free(querry);
        return rc;
    }  
    rc = sqlite3_bind_int(stmt,1,id);
    if(rc != SQLITE_OK){
        free(querry);
        return rc;
    }  
    rc = sqlite3_bind_double(stmt,2,value);
    if(rc != SQLITE_OK){
        free(querry);
        return rc;
    } 
    rc = sqlite3_bind_int64(stmt,3,(long int)ts);
    if(rc != SQLITE_OK){
        free(querry);
        return rc;
    } 

    rc = sqlite3_step(stmt);
    free(querry);
    if(rc != 101) //101 -> no more rows available, is logisch aangezien we gewoon een insert willen doen
        return rc;
    rc = sqlite3_finalize(stmt);
    if(rc != SQLITE_OK) return rc;
    return rc;//SQLITEOK = 0 -> rc is al de juiste waarde

}

int insert_sensor_from_file(DBCONN *conn, FILE *sensor_data){
    sensor_data_packed_t data_formatted;
    int rcB = SQLITE_OK;
    while(fread(&data_formatted,sizeof(sensor_data_packed_t),1,sensor_data)>0){        
        int rc = insert_sensor(conn,data_formatted.id,data_formatted.value,data_formatted.ts);   

        if(rc != SQLITE_OK){
            //failed to insert
            check_for_SQLOK(rc,conn,0);
            printf("%d",rc);
            rcB = rc;   
        }
    }
    return rcB;
}

int find_sensor_all(DBCONN *conn, callback_t f){
    char* querry;
    asprintf(&querry,"%s %s","Select * from", TO_STRING(TABLE_NAME));
    int rc = get_result_of_querry(conn,querry,f);
    free(querry);
    return rc;
}

int find_sensor_by_value(DBCONN *conn, sensor_value_t value, callback_t f){
    char* querry;
    asprintf(&querry,"%s %s %s", "Select * from", TO_STRING(TABLE_NAME),"where sensor_value = 15");
    int rc = get_result_of_querry(conn,querry,f);
    free(querry);
    return rc;
}

int find_sensor_exceed_value(DBCONN *conn, sensor_value_t value, callback_t f){
    char* querry;
    asprintf(&querry,"%s %s %s %f","Select * from",TO_STRING(TABLE_NAME),"where sensor_value > ",value);
    int rc = get_result_of_querry(conn,querry,f);
    free(querry);
    return rc;
}

int find_sensor_by_timestamp(DBCONN *conn, sensor_ts_t ts, callback_t f){
    char* querry;
    asprintf(&querry,"%s %s %s %ld","Select * from",TO_STRING(TABLE_NAME),"where timestamp =",ts);
    int r = get_result_of_querry(conn,querry,f);
    free(querry);
    return r;
}

int find_sensor_after_timestamp(DBCONN *conn, sensor_ts_t ts, callback_t f){
    char* querry;
    asprintf(&querry,"%s %s %s %ld","Select * from",TO_STRING(TABLE_NAME),"where timestamp >",ts);
    int r = get_result_of_querry(conn,querry,f);
    free(querry);
    return r;
}





