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
#include <pthread.h>    


int execute_sql_stmt(DBCONN* db, char* statement,int callback, char smt){
    char* err_msg = 0;
    int rc = sqlite3_exec(db,statement,0,0,&err_msg);

    if(rc != SQLITE_OK){
        DEBUG_PRINTF("Failed to fetch data: %s \n",err_msg);
        char* msg;
        asprintf(&msg,"Failed to fetch data: %s",err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        write_to_logger(msg);
        free(msg);
        exit(EXIT_FAILURE);
    }
    return rc;
}


DBCONN *init_connection(char clear_up_flag){

    DBCONN *db = NULL;
        
    int rc = sqlite3_open(TO_STRING(DB_NAME),&db);

    if(rc != SQLITE_OK){
        write_to_logger("Unable to connect to SQL server");
        sqlite3_close(db);
        ERROR_HANDLER(1,"Unalbe to connect to SQL server");
    }
    else{
        char* str = "Connection to SQL server established";
        write_to_logger(str);
    }

    if(clear_up_flag == 1){
        char* clear_tbl_stmt;
        asprintf(&clear_tbl_stmt,"%s %s %s","DROP TABLE IF EXISTS",TO_STRING(TABLE_NAME),";");
        execute_sql_stmt(db,clear_tbl_stmt,0,0);
        free(clear_tbl_stmt);

    }
    
    char* sql_stmt ;
    asprintf(&sql_stmt,"%s %s %s","CREATE TABLE IF NOT EXISTS",TO_STRING(TABLE_NAME),"(id INTEGER PRIMARY KEY AUTOINCREMENT, sensor_id	INTEGER NOT NULL, sensor_value NUMERIC NOT NULL, timestamp INTEGER NOT NULL);");
    execute_sql_stmt(db,sql_stmt,0,0);
    free(sql_stmt);
    char* buffer;
    asprintf(&buffer,"New table %s created",TO_STRING(TABLE_NAME));
    write_to_logger(buffer);
    free(buffer);
    
    return db;
}


void disconnect(DBCONN *conn){
    write_to_logger("Connection to SQL server lost(manually disconnected)");
    ERROR_HANDLER(sqlite3_close(conn)!=0,"Failed to close sql3 connection");
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





