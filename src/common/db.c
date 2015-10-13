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

int _db_initial()
{
    conn = mysql_init(NULL);
    if(!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PWD, DB_NAME, DB_PORT, NULL, 0))
    {
        LOG_ERROR("can't connect database: %s", DB_NAME);
        return -1;
    }
    LOG_INFO("connect database: %s", DB_NAME);
    return 0;
}

int _db_destruct()
{
    mysql_close(conn);
    LOG_INFO("destruct database: %s", DB_NAME);
    return 0;
}

//check whether the given table exists
//return 1 when exists, else 0
int _db_isTableCreated(const char* imeiName)
{
    MYSQL_RES *res;

    char reg[IMEI_LENGTH *2 + 5] = "gps_";
    strncat(reg, imeiName, IMEI_LENGTH * 2);
    if((res = mysql_list_tables(conn, reg)) == NULL)
    {
        LOG_ERROR("can't judge whether tables of IMEI:%s exist", imeiName);
    }

    int rows = mysql_num_rows(res);

    mysql_free_result(res);

    if(0 == rows)
    {
        return 0;
    }
    return 1;
}

int _db_createGPS(const char* tableName)
{
    char query[MAX_QUERY];
    //create table gps_IMEI(timestamp INT, lat DOUBLE, lon DOUBLE, speed TINYINT, course SMALLINT)
    snprintf(query, MAX_QUERY, "create table gps_%s(timestamp INT,lat DOUBLE(8,5),lon DOUBLE(8,5),speed TINYINT UNSIGNED,course SMALLINT,primary key(timestamp))", tableName);
    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't create table: gps_%s", tableName);
        return -1;
    }
    LOG_INFO("create table: gps_%s", tableName);
    return 0;
}

int _db_createCGI(const char* tableName)
{
    char query[MAX_QUERY * 3];
    //create table cgi_IMEI(timestamp INT, mcc SMALLINT, mnc SMALLINT, lac SMALLINT, ci SMALLINT, rxl SMALLINT)
    snprintf(query, MAX_QUERY * 3, "create table cgi_%s(timestamp INT,mcc SMALLINT,mnc SMALLINT,lac0 SMALLINT,ci0 SMALLINT,rxl0 SMALLINT,lac1 SMALLINT,ci1 SMALLINT,rxl1 SMALLINT,lac2 SMALLINT,ci2 SMALLINT,rxl2 SMALLINT,lac3 SMALLINT,ci3 SMALLINT,rxl3 SMALLINT,lac4 SMALLINT,ci4 SMALLINT,rxl4 SMALLINT,lac5 SMALLINT,ci5 SMALLINT,rxl5 SMALLINT,lac6 SMALLINT,ci6 SMALLINT,rxl6 SMALLINT)", tableName);
    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't create table: cgi_%s", tableName);
        return -1;
    }
    LOG_INFO("create table: cgi_%s", tableName);
    return 0;
}

int _db_saveGPS(const char *imeiName, int timestamp, float lat, float lon, char speed, short course)
{
    //timestamp INT, lat DOUBLE, lon DOUBLE, speed TINYINT, course SMALLINT
    char query[MAX_QUERY];
    snprintf(query, MAX_QUERY, "insert into gps_%s(timestamp,lat,lon,speed,course) values(%d,%f,%f,%u,%d)",imeiName, timestamp, lat, lon, speed, course);
    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't insert into gps_%s", imeiName);
        return -1;
    }
    LOG_INFO("insert into gps_%s: %d, %f, %f, %u, %d", imeiName, timestamp, lat, lon, speed, course);
    return 0;
}

int _db_saveCGI(const char *imeiName, int timestamp, int cellNo, const CGI_MC cell[])
{
    //prevent string of sql(insert) overflow
    char query[MAX_QUERY * 3];
    int i;
    char *current = query;
    int step = snprintf(current, MAX_QUERY, "insert into cgi_%s(timestamp,mcc,mnc,lac0,ci0,rxl0)", imeiName);
    for(i = 1; i < cellNo; ++i)
    {
        current += step - 1;
        step = snprintf(current, MAX_QUERY, ",lac%d,ci%d,rxl%d)", i, i, i);
    }
    current += step;
    step = snprintf(current, MAX_QUERY, " values(%d,%d,%d,%d,%d,%d)", timestamp, cell[0].mcc, cell[0].mnc, cell[0].lac, cell[0].ci, cell[0].rxl);
    for(i = 1; i < cellNo; ++i)
    {
        current += step - 1;
        step = snprintf(current, MAX_QUERY, ",%d,%d,%d)", cell[i].lac, cell[i].ci, cell[i].rxl);
    }
    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't insert into cgi_%s", imeiName);
        return -1;
    }
    LOG_INFO(query);
    return 0;
}

/*
int _db_saveCGI(const char *imeiName, int timestamp, short mcc, short mnc, int cellNo, short lac[], short ci[], short rxl[])
{
    //timestamp INT, mcc SMALLINT, mnc SMALLINT, lac SMALLINT, ci CHAR(3)
    char query[MAX_QUERY * 3]; //prevent data of cgi overflow
    int i;
    char *current = query;
    int step = snprintf(current, MAX_QUERY, "insert into cgi_%s(timestamp,mcc,mnc,lac0,ci0,rxl0)", imeiName);
    for(i = 1; i < cellNo; ++i)
    {
        current += step - 1;
        step = snprintf(current, MAX_QUERY, ",lac%d,ci%d,rxl%d)", i, i, i);
    }
    current += step;
    step = snprintf(current, MAX_QUERY, "values(%d,%d,%d,%d,%d,%d)", timestamp, mcc, mnc, lac[0], ci[0], rxl[0]);
    for(i = 1; i < cellNo; ++i)
    {
        current += step -1;
        step = snprintf(current, MAX_QUERY, ",%d,%d,%d)", lac[i], ci[i], rxl[i]);
    }
    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't insert into cgi_%s", imeiName);
        return -1;
    }
    LOG_INFO(query);
    return 0;
}

int _db_saveCGI(const char *imeiName, int timestamp, short mcc, short mnc, short lac, short ci, short rxl)
{
    //timestamp INT, mcc SMALLINT, mnc SMALLINT, lac SMALLINT, ci CHAR(3)
    char query[MAX_QUERY];
    snprintf(query, MAX_QUERY, "insert into cgi_%s(timestamp,mcc,mnc,lac,ci,rxl) values(%d,%d,%d,%d,%d,%d)", imeiName, timestamp, mcc, mnc, (unsigned short)lac, (unsigned short)ci, rxl);
    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't insert into cgi_%s", imeiName);
        return -1;
    }
    LOG_INFO("insert into cgi_%s: %d %d %d %d %d %d", imeiName, timestamp, mcc, mnc, (unsigned short)lac, (unsigned short)ci, rxl);
    return 0;
}
*/
/*Object db
Names of the table and columns need modifing*/

int _db_doWithOBJ(void (*func1)(const char*, int), void (*func2)(const char *))
{
    char query[] = "select imei, lastlogintime from object";
    if(mysql_query(conn, query))
    {
        LOG_FATAL("can't get objects from db");
        return -1;
    }
    MYSQL_RES *result;
    MYSQL_ROW row;
    result = mysql_use_result(conn);
    while(row= mysql_fetch_row(result))
    {
        func1(row[0], atoi(row[1]));
        func2(row[0]);
    }
    mysql_free_result(result);
    return 0;
}

int _db_insertOBJ(const char *imeiName, int lastLoginTime)
{
    char query[MAX_QUERY];
    snprintf(query, MAX_QUERY, "insert into object(imei, lastlogintime) values(\'%s\', %d)", imeiName, lastLoginTime);
    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't insert %s into object", imeiName);
        return -1;
    }
    return 0;
}

int _db_updateOBJ(const char *imeiName, int lastLoginTime)
{
    char query[MAX_QUERY];
    snprintf(query, MAX_QUERY, "update object set lastlogintime = %d where imei = \'%s\'", lastLoginTime, imeiName);
    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't update Obj where imei = %s", imeiName);
        return -1;
    }
    return 0;
}


int db_initial()
{
#ifdef WITH_DB
    return _db_initial();
#endif

    return 0;
}
int db_destruct()
{
#ifdef WITH_DB
    return _db_destruct();
#endif

    return 0;
}

int db_isTableCreated(const char* imeiName)
{
#ifdef WITH_DB
    return _db_isTableCreated(imeiName);
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

int db_saveGPS(const char* imeiName, int timestamp, float lat, float lon, char speed, short course)
{
#ifdef WITH_DB
    return _db_saveGPS(imeiName, timestamp, lat, lon, speed, course);
#endif

    return 0;
}

int db_saveCGI(const char* imeiName, int timestamp, int cellNo, const CGI_MC cell[])
{
#ifdef WITH_DB
    return _db_saveCGI(imeiName, timestamp, cellNo, cell);
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

int db_insertOBJ(const char *imeiName, int lastlogintime)
{
#ifdef WITH_DB
    return _db_insertOBJ(imeiName, lastlogintime);
#endif

    return 0;
}

int db_updateOBJ(const char *imeiName, int lastLoginTime)
{
#ifdef WITH_DB
    return _db_updateOBJ(imeiName, lastLoginTime);
#endif

    return 0;
}
