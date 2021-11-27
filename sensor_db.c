#include "sensor_db.h"
#include <string.h>

void prints(char* string){
    printf("%s\n",string);
}

int execute_sql_stmt(DBCONN* db, char* statement,int callback, char smt){
    char* err_msg = 0;
    int rc = sqlite3_exec(db,statement,0,0,&err_msg);

    if(rc != SQLITE_OK){
        printf("Failed to fetch data: %s \n",err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    return rc;
}

int check_for_SQLOK(int rc,DBCONN* conn,char interrupt_on_error){
    if(rc != SQLITE_OK){
        printf("Problem loading the data:%s \n",sqlite3_errmsg(conn));
        if(interrupt_on_error == 1)
            exit(EXIT_FAILURE);
    }
    return rc;
}

DBCONN *init_connection(char clear_up_flag){
    DBCONN *db = NULL;

    
    int rc = sqlite3_open(TO_STRING(DB_NAME),&db);

    if(rc != SQLITE_OK){
        printf("Couldn't open the database\n");
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }

    if(clear_up_flag == 1){
        char* clear_tbl_stmt = "DROP TABLE IF EXISTS "
                        TO_STRING(TABLE_NAME)
                        ";\n";
        execute_sql_stmt(db,clear_tbl_stmt,0,0);

    }
    
    char* sql_stmt = "CREATE TABLE IF NOT EXISTS " 
            TO_STRING(TABLE_NAME) "(id INTEGER PRIMARY KEY AUTOINCREMENT, sensor_id	INTEGER NOT NULL, sensor_value NUMERIC NOT NULL, timestamp INTEGER NOT NULL);";
    
    
    execute_sql_stmt(db,sql_stmt,0,0);
    
    return db;
}

void disconnect(DBCONN *conn){
    sqlite3_close(conn);
    return;
}

int insert_sensor(DBCONN *conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts){
    char* query = "INSERT INTO " TO_STRING(TABLE_NAME) "(sensor_id,sensor_value,timestamp) VALUES(?,?,?)";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(conn,query,-1,&stmt,0);
    if(rc != SQLITE_OK) return rc;
    rc = sqlite3_bind_int(stmt,1,id);
    if(rc != SQLITE_OK) return rc;
    rc = sqlite3_bind_double(stmt,2,value);
    if(rc != SQLITE_OK) return rc;
    rc = sqlite3_bind_int64(stmt,3,(long int)time);
    if(rc != SQLITE_OK) return rc;

    rc = sqlite3_step(stmt);
    if(rc != 101) //101 -> no more rows available, is logisch aangezien we gewoon een insert willen doen
        return rc;
    rc = sqlite3_finalize(stmt);
    if(rc != SQLITE_OK) return rc;
    // prints("Inserted data");   
    return rc;//SQLITEOK = 0 -> rc is al de juiste waarde

}

int insert_sensor_from_file(DBCONN *conn, FILE *sensor_data){
    sensor_data_packed_t data_formatted;
    int rcB = SQLITE_OK;
    while(fread(&data_formatted,sizeof(sensor_data_packed_t),1,sensor_data)>0){
        int rc = 1;
        int amount_of_tries = 0;
        while(rc != 0 && amount_of_tries<5){
            //if fails once, try a few more times
            rc = insert_sensor(conn,data_formatted.id,data_formatted.value,data_formatted.ts);   
            amount_of_tries ++;
        }
        if(rc != SQLITE_OK){
            //doesn't work even after multiple tries
            check_for_SQLOK(rc,conn,0);
            printf("%d",rc);
            rcB = rc;   
        }
    }
    return rcB;
}



