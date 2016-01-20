/* 
 * File:   db.c
 * Author: jk
 *
 * Created on June 16, 2015, 9:10 AM
 */

#include <mysql/mysql.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "db.h"
#include "log.h"
#include "macro.h"

static MYSQL *conn = NULL;

static int _db_initial()
{
    char value = 1;

    conn = mysql_init(NULL);

    mysql_options(conn, MYSQL_OPT_RECONNECT, (char *)&value);

    if(!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PWD, DB_NAME, DB_PORT, NULL, 0))
    {
        LOG_ERROR("can't connect database: %s(%u, %s)", DB_NAME, mysql_errno(conn), mysql_error(conn));
        return -1;
    }
    LOG_INFO("connect database: %s", DB_NAME);
    return 0;
}

static int _db_destruct()
{
    mysql_close(conn);
    LOG_INFO("destruct database: %s", DB_NAME);
    return 0;
}

//check whether the given table exists
//return 1 when exists, else 0
static int _db_isTableCreated(const char* imeiName, int *num)
{
    MYSQL_RES *res;

    char reg[IMEI_LENGTH + 5] = "gps_";
    strncat(reg, imeiName, IMEI_LENGTH + 1);

    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if((res = mysql_list_tables(conn, reg)) == NULL)
    {
        LOG_ERROR("can't list tables of %s(%u, %s)", reg, mysql_errno(conn), mysql_error(conn));
        return 2;
    }

    *num = mysql_num_rows(res);

    mysql_free_result(res);

    return 0;
}

static int _db_createGPS(const char* tableName)
{
    char query[MAX_QUERY];
    
    snprintf(query, MAX_QUERY, "create table gps_%s(timestamp INT,lat DOUBLE(9,6),lon DOUBLE(9,6),altitude DOUBLE(9,6),speed DOUBLE(9,6),course DOUBLE(9,6),primary key(timestamp))", tableName);
    
    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't create table: gps_%s(%u, %s)", tableName, mysql_errno(conn), mysql_error(conn));
        return 2;
    }
    LOG_INFO("create table: gps_%s", tableName);

    return 0;
}

static int _db_createCGI(const char* tableName)
{
    char query[MAX_QUERY];
    //create table cgi_IMEI(timestamp INT, mcc SMALLINT, mnc SMALLINT, lac0 SMALLINT, ci0 SMALLINT, rxl0 SMALLINT...)
    snprintf(query, MAX_QUERY, "create table cgi_%s(timestamp INT,mcc SMALLINT,mnc SMALLINT,lac0 SMALLINT,ci0 SMALLINT,rxl0 SMALLINT,lac1 SMALLINT,ci1 SMALLINT,rxl1 SMALLINT,lac2 SMALLINT,ci2 SMALLINT,rxl2 SMALLINT,lac3 SMALLINT,ci3 SMALLINT,rxl3 SMALLINT,lac4 SMALLINT,ci4 SMALLINT,rxl4 SMALLINT,lac5 SMALLINT,ci5 SMALLINT,rxl5 SMALLINT,lac6 SMALLINT,ci6 SMALLINT,rxl6 SMALLINT)", tableName);
    
    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't create table: cgi_%s(%u, %s)", tableName, mysql_errno(conn), mysql_error(conn));
        return 2;
    }
    LOG_INFO("create table: cgi_%s", tableName);

    return 0;
}

static int _db_saveGPS(const char *imeiName, int timestamp, float lat, float lon, float altitude, float speed, float course)
{
    //timestamp INT, lat DOUBLE, lon DOUBLE, altitude DOUBLE, speed DOUBLE, course DOUBLE
    char query[MAX_QUERY];
    snprintf(query, MAX_QUERY, "insert into gps_%s(timestamp,lat,lon,altitude,speed,course) values(%d,%f,%f,%f,%f,%f)",imeiName, timestamp, lat, lon, altitude, speed, course);
    
    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't insert into gps_%s(%u, %s)", imeiName, mysql_errno(conn), mysql_error(conn));
        return 2;
    }

    LOG_INFO("insert into gps_%s: %d, %f, %f, %f, %f, %f", imeiName, timestamp, lat, lon, altitude, speed, course);
    return 0;
}

static int _db_saveCGI(const char *imeiName, int timestamp, const CGI_MC cell[], int cellNo)
{
    char query[MAX_QUERY];
    int i;
    char *current = query;
    int step = snprintf(current, MAX_QUERY, "insert into cgi_%s(timestamp,mcc,mnc,lac0,ci0,rxl0)", imeiName);
    for(i = 1; i < cellNo; ++i)
    {
        current += step - 1;
        step = snprintf(current, MAX_QUERY, ",lac%d,ci%d,rxl%d)", i, i, i);
    }
    current += step;
    step = snprintf(current, MAX_QUERY, " values(%d,%d,%d,%d,%d,%d)", timestamp, cell[0].mcc, cell[0].mnc, (unsigned short)cell[0].lac, (unsigned short)cell[0].ci, cell[0].rxl);
    for(i = 1; i < cellNo; ++i)
    {
        current += step - 1;
        step = snprintf(current, MAX_QUERY, ",%d,%d,%d)", (unsigned short)cell[i].lac, (unsigned short)cell[i].ci, cell[i].rxl);
    }

    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't insert into cgi_%s(%u, %s)", imeiName, mysql_errno(conn), mysql_error(conn));
        return 2;
    }
    LOG_INFO(query);
    return 0;
}

/*Object db
Names of the table and columns need modifing*/
static int _db_doWithOBJ(void (*func1)(const char*), void (*func2)(const char *))
{
    char query[] = "select imei from object";

    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if(mysql_query(conn, query))
    {
        LOG_FATAL("can't get objects from db(%u, %s)", mysql_errno(conn), mysql_error(conn));
        return 2;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;
    result = mysql_store_result(conn);
    while(row = mysql_fetch_row(result))
    {
        func1(row[0]); //obj_initial
        func2(row[0]); //mqtt_subscribe
    }
    mysql_free_result(result);
    return 0;
}

static int _db_insertOBJ(const char *imeiName, int ObjectType)
{
    char query[MAX_QUERY];
    snprintf(query, MAX_QUERY, "insert into object(imei, ObjectType) values(\'%s\', %d)", imeiName, ObjectType);
    
    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't insert %s into object(%u, %s)", imeiName, mysql_errno(conn), mysql_error(conn));
        return 2;
    }
    return 0;
}

static int _db_updateOBJIsPosted(const char *imeiName)
{
    char query[MAX_QUERY];
    snprintf(query, MAX_QUERY, "update object set IsPosted=1 where imei=%s", imeiName);

    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)", mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't update IsPosted where imei=%s(%u, %s)", imeiName, mysql_errno(conn), mysql_error(conn));
        return 2;
    }

    return 0;
}

static int _db_ResaveOBJUnpostedImei_cb(void (*func1)(const char*))
{
    char query[] = "select imei from object where IsPosted=0";

    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)", mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if(mysql_query(conn, query))
    {
        LOG_FATAL("can't get objects from db(%u, %s)", mysql_errno(conn), mysql_error(conn));
        return 2;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;
    result = mysql_store_result(conn);
    while(row = mysql_fetch_row(result))
    {
        LOG_INFO("leancloud_saveDid: %s", row[0]);

        func1(row[0]); //leancloud_saveDid
    }
    mysql_free_result(result);

    return 0;
}

int db_initial(void)
{
#ifdef WITH_DB
    return _db_initial();
#endif

    return 0;
}
int db_destruct(void)
{
#ifdef WITH_DB
    return _db_destruct();
#endif

    return 0;
}

int db_isTableCreated(const char* imeiName, int *num)
{
#ifdef WITH_DB
    return _db_isTableCreated(imeiName, num);
#endif

    return 0;
}

int db_createGPS(const char* tableName)
{
#ifdef WITH_DB
    return _db_createGPS(tableName);
#endif

    return 0;
}

int db_createCGI(const char* tableName)
{
#ifdef WITH_DB
    return _db_createCGI(tableName);
#endif

    return 0;
}

int db_saveGPS(const char* imeiName, int timestamp, float lat, float lon, float altitude, float speed, float course)
{
#ifdef WITH_DB
    return _db_saveGPS(imeiName, timestamp, lat, lon, altitude, speed, course);
#endif

    return 0;
}

int db_saveCGI(const char* imeiName, int timestamp, const CGI_MC cell[], int cellNo)
{
#ifdef WITH_DB
    return _db_saveCGI(imeiName, timestamp, cell, cellNo);
#endif

    return 0;
}

int db_doWithOBJ(void (*func)(const char*, int), void (*func2)(const char *))
{
#ifdef WITH_DB
    return _db_doWithOBJ(func, func2);
#endif

    return 0;
}

int db_insertOBJ(const char *imeiName, int ObjectType)
{
#ifdef WITH_DB
    return _db_insertOBJ(imeiName, ObjectType);
#endif

    return 0;
}

int db_updateOBJIsPosted(const char *imeiName)
{
#ifdef WITH_DB
    return _db_updateOBJIsPosted(imeiName);
#endif

    return 0;
}

int db_ResaveOBJUnpostedImei_cb(void (*func1)(const char*))
{
#ifdef WITH_DB
    return _db_ResaveOBJUnpostedImei_cb(func1);
#endif

    return 0;
}
