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
#include "object.h"

static MYSQL *conn = NULL;

static int _db_initial()
{
    char value = 1;

    conn = mysql_init(NULL);

    mysql_options(conn, MYSQL_OPT_RECONNECT, (char *)&value);

    if(!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PWD, NULL, DB_PORT, NULL, 0))
    {
        LOG_ERROR("can't connect to mysql(%u, %s)", mysql_errno(conn), mysql_error(conn));
        return -1;
    }
    else
    {
        /* create database if not exists gps */
        char query[MAX_QUERY];
        snprintf(query, MAX_QUERY, "create database if not exists %s", DB_NAME);

        if(mysql_ping(conn))
        {
            LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
            return 1;
        }

        if(mysql_query(conn, query))
        {
            LOG_ERROR("can't create database %s(%u, %s)", DB_NAME, mysql_errno(conn), mysql_error(conn));
            return 2;
        }

        /* use gps */
        snprintf(query, MAX_QUERY, "use %s", DB_NAME);

        if(mysql_ping(conn))
        {
            LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
            return 1;
        }

        if(mysql_query(conn, query))
        {
            LOG_ERROR("can't use %s(%u, %s)", DB_NAME, mysql_errno(conn), mysql_error(conn));
            return 2;
        }

        /* creat table object if not exists */
        snprintf(query, MAX_QUERY,
        "create table if not exists object( \
        imei char(16) not null primary key, \
        RegisterTime timestamp default CURRENT_TIMESTAMP, \
        IsPosted tinyint default 0, \
        ObjectType int(4) not null, \
        voltage tinyint default 0, \
        itinerary bigint default 0, \
        ccid char(20), \
        imsi char(15) \
        )");

        if(mysql_ping(conn))
        {
            LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
            return 1;
        }

        if(mysql_query(conn, query))
        {
            LOG_ERROR("can't creat table object(%u, %s)", mysql_errno(conn), mysql_error(conn));
            return 2;
        }

        /* creat table imei2objectID if not exists */
        snprintf(query, MAX_QUERY, "create table if not exists imei2objectID(imei char(15) not null primary key, \
                                    objectID char(24) not null)");

        if(mysql_ping(conn))
        {
            LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
            return 1;
        }

        if(mysql_query(conn, query))
        {
            LOG_ERROR("can't creat table imei2objectID(%u, %s)", mysql_errno(conn), mysql_error(conn));
            return 2;
        }

        /* creat table imei2objectID if not exists */
        snprintf(query, MAX_QUERY, "create table if not exists imei2Telnumber(imei char(15) not null primary key, \
        Telnumber varchar(12) not null,\
        Callnumber varchar(12))");

        if(mysql_ping(conn))
        {
            LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
            return 1;
        }

        if(mysql_query(conn, query))
        {
            LOG_ERROR("can't creat table imei2Telnumber(%u, %s)", mysql_errno(conn), mysql_error(conn));
            return 2;
        }

        /* creat table FirmwarePkg if not exists */
        snprintf(query, MAX_QUERY, "create table if not exists FirmwarePkg(version bigint not null primary key, \
        fileName varchar(128) not null)");

        if(mysql_ping(conn))
        {
            LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
            return 1;
        }

        if(mysql_query(conn, query))
        {
            LOG_ERROR("can't creat table FirmwarePkg(%u, %s)", mysql_errno(conn), mysql_error(conn));
            return 2;
        }

        /* creat table AppPackage if not exists */
        snprintf(query, MAX_QUERY, "create table if not exists AppPackage(id bigint not null primary key auto_increment, \
        versionName varchar(32) not null, \
        versionCode bigint default 0, \
        changeLog varchar(256), \
        fileName varchar(32), \
        type int default 0)");

        if(mysql_ping(conn))
        {
            LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
            return 1;
        }

        if(mysql_query(conn, query))
        {
            LOG_ERROR("can't creat table AppPackage(%u, %s)", mysql_errno(conn), mysql_error(conn));
            return 2;
        }

        /* creat table log if not exists */
        snprintf(query, MAX_QUERY, "create table if not exists log(time timestamp default CURRENT_TIMESTAMP, \
        imei char(15) not null, \
        event char(16) not null)");

        if(mysql_ping(conn))
        {
            LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
            return 1;
        }

        if(mysql_query(conn, query))
        {
            LOG_ERROR("can't creat table log(%u, %s)", mysql_errno(conn), mysql_error(conn));
            return 2;
        }

        /* creat table motorcycle if not exists */
        snprintf(query, MAX_QUERY, "create table if not exists motorcycle(imei varchar(15) not null primary key)");

        if(mysql_ping(conn))
        {
            LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
            return 1;
        }

        if(mysql_query(conn, query))
        {
            LOG_ERROR("can't creat table log(%u, %s)", mysql_errno(conn), mysql_error(conn));
            return 2;
        }

        LOG_INFO("create and use database %s; creat table object, imei2objectID and log", DB_NAME);
        return 0;
    }

#if 0
    if(!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PWD, DB_NAME, DB_PORT, NULL, 0))
    {
        if(mysql_errno(conn) == 1049)
        {
            /* (1049, Unknown database 'gps'), connect to default database and creat database gps */

            if(!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PWD, NULL, DB_PORT, NULL, 0))
            {
                LOG_ERROR("can't connect database: NULL(%u, %s)", mysql_errno(conn), mysql_error(conn));
                return -1;
            }
            else
            {
                /* create database gps */
                char query[MAX_QUERY];
                snprintf(query, MAX_QUERY, "create database %s", DB_NAME);

                if(mysql_ping(conn))
                {
                    LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
                    return 1;
                }

                if(mysql_query(conn, query))
                {
                    LOG_ERROR("can't create database %s(%u, %s)", DB_NAME, mysql_errno(conn), mysql_error(conn));
                    return 2;
                }

                /* use gps */
                snprintf(query, MAX_QUERY, "use %s", DB_NAME);

                if(mysql_ping(conn))
                {
                    LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
                    return 1;
                }

                if(mysql_query(conn, query))
                {
                    LOG_ERROR("can't use %s(%u, %s)", DB_NAME, mysql_errno(conn), mysql_error(conn));
                    return 2;
                }

                /* creat table object if not exists */
                snprintf(query, MAX_QUERY, "create table if not exists object(imei char(16) not null primary key, \
                                            RegisterTime timestamp default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP, \
                                            IsPosted tinyint default '0', \
                                            ObjectType int(4) not null)");

                if(mysql_ping(conn))
                {
                    LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
                    return 1;
                }

                if(mysql_query(conn, query))
                {
                    LOG_ERROR("can't creat table object(%u, %s)", mysql_errno(conn), mysql_error(conn));
                    return 2;
                }

                /* creat table imei2objectID if not exists */
                snprintf(query, MAX_QUERY, "create table if not exists imei2objectID(imei char(15) not null primary key, \
                                            objectID char(24) not null)");

                if(mysql_ping(conn))
                {
                    LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
                    return 1;
                }

                if(mysql_query(conn, query))
                {
                    LOG_ERROR("can't creat table imei2objectID(%u, %s)", mysql_errno(conn), mysql_error(conn));
                    return 2;
                }

                LOG_INFO("create and use database: %s, creat table object, creat table imei2objectID", DB_NAME);
                return 0;
            }
        }
        else
        {
            LOG_ERROR("can't connect database: %s(%u, %s)", DB_NAME, mysql_errno(conn), mysql_error(conn));
            return -1;
        }
    }
    else
    {
        LOG_INFO("connect database: %s", DB_NAME);

        /* creat table object if not exists */
        char query[MAX_QUERY];
        snprintf(query, MAX_QUERY, "create table if not exists object(imei char(16) not null primary key, \
                                    RegisterTime timestamp default CURRENT_TIMESTAMP , \
                                    IsPosted tinyint default '0', \
                                    ObjectType int(4) not null)");

        if(mysql_ping(conn))
        {
            LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
            return 1;
        }

        if(mysql_query(conn, query))
        {
            LOG_ERROR("can't creat table object(%u, %s)", mysql_errno(conn), mysql_error(conn));
            return 2;
        }

        /* creat table imei2objectID if not exists */
        snprintf(query, MAX_QUERY, "create table  if not exists imei2objectID(imei char(15) not null primary key, \
                                    objectID char(24) not null)");

        if(mysql_ping(conn))
        {
            LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
            return 1;
        }

        if(mysql_query(conn, query))
        {
            LOG_ERROR("can't creat table imei2objectID(%u, %s)", mysql_errno(conn), mysql_error(conn));
            return 2;
        }

        /* creat table log if not exists */
        snprintf(query, MAX_QUERY, "create table  if not exists log(imei char(15) not null primary key, \
                                    objectID char(24) not null)");

        if(mysql_ping(conn))
        {
            LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
            return 1;
        }

        if(mysql_query(conn, query))
        {
            LOG_ERROR("can't creat table imei2objectID(%u, %s)", mysql_errno(conn), mysql_error(conn));
            return 2;
        }

        return 0;
    }
#endif
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
    MYSQL_RES *result;
    MYSQL_ROW row;

    char reg[IMEI_LENGTH + 5] = "%_";
    strncat(reg, imeiName, IMEI_LENGTH + 1);

    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if((result = mysql_list_tables(conn, reg)) == NULL)
    {
        LOG_ERROR("can't list tables of %s(%u, %s)", reg, mysql_errno(conn), mysql_error(conn));
        return 2;
    }
    *num = mysql_num_rows(result);

    mysql_free_result(result);
    return 0;
}

static int _db_createGPS(const char* tableName)
{
    char query[MAX_QUERY];

    snprintf(query, MAX_QUERY, "create table if not exists gps_%s(timestamp INT,lat DOUBLE(9,6),lon DOUBLE(9,6),speed TINYINT default 0,course SMALLINT default 0,primary key(timestamp))", tableName);

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
    LOG_INFO("create table if not exists: gps_%s", tableName);

    return 0;
}

static int _db_createCGI(const char* tableName)
{
    char query[MAX_QUERY];
    //create table cgi_IMEI(timestamp INT, mcc SMALLINT, mnc SMALLINT, lac0 SMALLINT, ci0 SMALLINT, rxl0 SMALLINT...)
    snprintf(query, MAX_QUERY, "create table if not exists cgi_%s(timestamp INT,mcc SMALLINT,mnc SMALLINT,lac0 SMALLINT,ci0 SMALLINT,rxl0 SMALLINT,lac1 SMALLINT,ci1 SMALLINT,rxl1 SMALLINT,lac2 SMALLINT,ci2 SMALLINT,rxl2 SMALLINT,lac3 SMALLINT,ci3 SMALLINT,rxl3 SMALLINT,lac4 SMALLINT,ci4 SMALLINT,rxl4 SMALLINT,lac5 SMALLINT,ci5 SMALLINT,rxl5 SMALLINT,lac6 SMALLINT,ci6 SMALLINT,rxl6 SMALLINT)", tableName);

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
    LOG_INFO("create table if not exists: cgi_%s", tableName);

    return 0;
}

static int _db_createiItinerary(const char* tableName)
{
    char query[MAX_QUERY];
    //create table cgi_IMEI(timestamp INT, mcc SMALLINT, mnc SMALLINT, lac0 SMALLINT, ci0 SMALLINT, rxl0 SMALLINT...)
    snprintf(query, MAX_QUERY, "create table if not exists itinerary_%s(CreateAt timestamp default CURRENT_TIMESTAMP,starttime INT not NULL,startlat DOUBLE(9,6),startlon DOUBLE(9,6),endtime INT not NULL,endlat DOUBLE(9,6),endlon DOUBLE(9,6),itinerary INT default 0)", tableName);

    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't create table: itinerary_%s(%u, %s)", tableName, mysql_errno(conn), mysql_error(conn));
        return 2;
    }
    LOG_INFO("create table if not exists: itinerary_%s", tableName);

    return 0;
}


static int _db_getHistoryGPS(const char *imeiName, int starttime, int endtime, void *action, void *userdata)
{
    char speed = 0;
    short course = 0;
    ONEGPS_PROC fun = action;
    char query[MAX_QUERY] = {0};

    snprintf(query,MAX_QUERY,"select * from gps_%s where timestamp >= %d and timestamp<= %d", imeiName, starttime, endtime);
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
        if(!row[0] || !row[1] || !row[2])
        {
            LOG_ERROR("There is somerow as timestamp or lat or lon is null:%s", imeiName);
            continue;
        }

        if(row[3])
        {
            speed = atoi(row[3]);
        }
        else
        {
            speed = 0;
        }

        if(row[4])
        {
            course = atoi(row[4]);
        }
        else
        {
            course = 0;
        }

        fun(atoi(row[0]), atof(row[1]), atof(row[2]), speed, course, userdata);

    }
    mysql_free_result(result);
    return 0;
}

static int _db_getGPS(const char *imeiName, void *action, void *userdata)
{
    char speed = 0;
    short course = 0;
    ONEGPS_PROC fun = action;
    char query[MAX_QUERY] = {0};

    snprintf(query,MAX_QUERY,"select * from gps_%s order by timestamp desc limit 1", imeiName);
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
        if(!row[0] || !row[1] || !row[2])
        {
            LOG_ERROR("There is somerow as timestamp or lat or lon is null:%s", imeiName);
            continue;
        }

        if(row[3])
        {
            speed = atoi(row[3]);
        }
        else
        {
            speed = 0;
        }

        if(row[4])
        {
            course = atoi(row[4]);
        }
        else
        {
            course = 0;
        }

        fun(atoi(row[0]), atof(row[1]), atof(row[2]), speed, course, userdata);

    }
    mysql_free_result(result);
    return 0;
}

static int _db_getiItinerary(const char *imeiName, int starttime, int endtime, void *action, void *userdata)
{
    ONEITINERARY_PROC fun = action;
    char query[MAX_QUERY] = {0};

    snprintf(query,MAX_QUERY,"select * from itinerary_%s where starttime >= %d and endtime<= %d", imeiName, starttime, endtime);
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
        if(!row[1] || !row[2] || !row[3] || !row[4] || !row[5] || !row[6] || !row[7])
        {
            LOG_ERROR("There is somerow is null:%s", imeiName);
            continue;
        }
        fun(atoi(row[1]),atof(row[2]),atof(row[3]),atoi(row[4]),atof(row[5]),atof(row[6]),atoi(row[7]),userdata);
    }

    mysql_free_result(result);
    return 0;
}

static int _db_deleteTelNumber(const char *imeiName)
{
    char query[MAX_QUERY] = {0};

    snprintf(query,MAX_QUERY,"delete from imei2Telnumber where imei = \'%s\'", imeiName);
    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }
    if(mysql_query(conn, query))
    {
        LOG_FATAL("can't delete from imei2Telnumber(%u, %s)", mysql_errno(conn), mysql_error(conn));
        return 2;
    }
    return 0;
}

static int _db_replaceTelNumber(const char *imeiName, const char *telNumber)
{
    char query[MAX_QUERY] = {0};

    snprintf(query,MAX_QUERY,"replace into imei2Telnumber(imei,Telnumber) values(\'%s\',\'%s\')", imeiName, telNumber);
    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }
    if(mysql_query(conn, query))
    {
        LOG_FATAL("can't insert number into db(%u, %s)", mysql_errno(conn), mysql_error(conn));
        return 2;
    }
    return 0;
}

static int _db_updateCallNumber(const char *imeiName, const char *CallNumber)
{
    char query[MAX_QUERY] = {0};

    snprintf(query,MAX_QUERY,"update imei2Telnumber set Callnumber=\'%s\' where imei=\'%s\'", CallNumber, imeiName);
    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }
    if(mysql_query(conn, query))
    {
        LOG_FATAL("can't insert number into db(%u, %s)", mysql_errno(conn), mysql_error(conn));
        return 2;
    }
    return 0;
}

static int _db_getTelNumber(const char *imeiName, char *telNumber, char *callNumber)
{
#define MAX_TELNUMBER_LEN 12
    char query[MAX_QUERY] = {0};

    snprintf(query,MAX_QUERY,"select Telnumber,Callnumber from imei2Telnumber where imei = \'%s\'", imeiName);
    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }
    if(mysql_query(conn, query))
    {
        LOG_FATAL("can't select Telnumber from into db(%u, %s)", mysql_errno(conn), mysql_error(conn));
        return 2;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;
    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);

    if(!row)
    {
        LOG_DEBUG("No telNumber(%s) in imei2Telnumber",imeiName);
        return 3;
    }

    if(row[0])
    {
        strncpy(telNumber,row[0],MAX_TELNUMBER_LEN);
    }

    if(row[1])
    {
        strncpy(callNumber,row[1],MAX_TELNUMBER_LEN);
    }

    return 0;
}

static int _db_saveGPS(const char *imeiName, int timestamp, float lat, float lon, char speed, short course)
{
    //timestamp INT, lat DOUBLE, lon DOUBLE, speed TINYINT, course SMALLINT
    char query[MAX_QUERY];
    snprintf(query, MAX_QUERY, "insert into gps_%s(timestamp,lat,lon,speed,course) values(%d,%f,%f,%d,%d)",imeiName, timestamp, lat, lon, speed, course);

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

    LOG_INFO("insert into gps_%s: %d, %f, %f, %d, %d", imeiName, timestamp, lat, lon, speed, course);
    return 0;
}

static int _db_getLastGPS(OBJECT *obj)
{
    char query[MAX_QUERY];

    snprintf(query, MAX_QUERY, "select * from gps_%s order by timestamp desc limit 1", obj->IMEI);
    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't select gps_%s(%u, %s)", obj->IMEI, mysql_errno(conn), mysql_error(conn));
        return 2;
    }

    MYSQL_ROW row;
    MYSQL_RES *result;

    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);
    if(!row)
    {
        LOG_INFO("NO gps data in gps_%s database.", obj->IMEI);
        return 3;
    }

    if(row[0])
    {
        obj->timestamp = atoi(row[0]);
    }
    else
    {
        LOG_ERROR("timestamp is null:%s", obj->IMEI);
        mysql_free_result(result);
        return 4;
    }

    if(row[1])
    {
        obj->lat = atof(row[1]);
    }
    else
    {
        LOG_ERROR("gps->lat is null:%s", obj->IMEI);
        mysql_free_result(result);
        return 4;
    }

    if(row[2])
    {
        obj->lon = atof(row[2]);
    }
    else
    {
        LOG_ERROR("gps->lon is null:%s", obj->IMEI);
        mysql_free_result(result);
        return 4;
    }

    if(row[3])
    {
        obj->speed= atoi(row[3]);
    }
    else
    {
        obj->speed= 0;
    }

    if(row[4])
    {
        obj->course= atoi(row[4]);
    }
    else
    {
        obj->course= 0;
    }

    mysql_free_result(result);

    return 0;
}

static int _db_saveiItinerary(const char* tableName, int starttime, float startlat, float startlon, int endtime, float endlat, float endlon, int itinerary)
{
    //timestamp INT, lat DOUBLE, lon DOUBLE, speed TINYINT, course SMALLINT
    char query[MAX_QUERY];
    snprintf(query, MAX_QUERY, "insert into itinerary_%s(starttime,startlat,startlon,endtime,endlat,endlon,itinerary) values(%d,%f,%f,%d,%f,%f,%d)",tableName, starttime, startlat, startlon, endtime, endlat, endlon, itinerary);

    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't insert into itinerary_%s(%u, %s)", tableName, mysql_errno(conn), mysql_error(conn));
        return 2;
    }

    LOG_INFO("insert into itinerary_%s: starttime(%d), endtime(%d), itinerary(%d)", tableName, starttime, endtime, itinerary);
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

    return 0;
}

static int _db_doWithOBJ(void (*func1)(const char*), void (*func2)(const char *), int ObjectType)
{
#define MAX_QUERY_LEN 128
    char imei[IMEI_LENGTH];
    char query[MAX_QUERY_LEN] = {0};

    snprintf(query,MAX_QUERY_LEN,"select imei from object where length(imei)=15 and ObjectType=%d",ObjectType);

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
        memcpy(imei, row[0], IMEI_LENGTH);

        func1(imei); //obj_initial
        func2(imei); //mqtt_subscribe
    }
    mysql_free_result(result);
    return 0;
}

static int _db_insertOBJ(const char *imeiName, int ObjectType, char voltage)
{
    char query[MAX_QUERY];
    snprintf(query, MAX_QUERY, "insert into object(imei, ObjectType, voltage) values(\'%s\', %d, %d)", imeiName, ObjectType, voltage);

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

static int _db_updateSimInfo(const char *imeiName, const char *ccid, const char *imsi)
{
    char query[MAX_QUERY];
    snprintf(query, MAX_QUERY, "update object set ccid = \'%s\' , imsi = \'%s\' where imei =\'%s\'", ccid, imsi, imeiName);

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

static int _db_doWithObjectID(int (*func1)(const char*, const char*))
{
    char query[] = "select * from imei2objectID where length(imei)=15";

    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if(mysql_query(conn, query))
    {
        LOG_FATAL("can't get imei2objectID from db(%u, %s)", mysql_errno(conn), mysql_error(conn));
        return 2;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;
    result = mysql_store_result(conn);
    while(row = mysql_fetch_row(result))
    {
        func1(row[0], row[1]); //objectID_add_hash
    }
    mysql_free_result(result);
    return 0;
}

static int _db_add_ObjectID(const char *imei, const char *objectID)
{
    char query[MAX_QUERY];
    snprintf(query, MAX_QUERY, "insert into imei2objectID(imei, objectID) values(\'%s\', \'%s\')", imei, objectID);

    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't add imei(%s), objectID(%s) into imei2objectID(%u, %s)", imei, objectID, mysql_errno(conn), mysql_error(conn));
        return 2;
    }
    return 0;
}

static int _db_add_log(const char *imei, const char *event)
{
    char query[MAX_QUERY];
    snprintf(query, MAX_QUERY, "insert into log(imei, event) values(\'%s\', \'%s\')", imei, event);

    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 1;
    }

    if(mysql_query(conn, query))
    {
        LOG_ERROR("can't add imei(%s), event(%d) into log(%u, %s)", imei, event, mysql_errno(conn), mysql_error(conn));
        return 2;
    }
    return 0;
}

static int _db_updateItinerary(const char *imeiName, long itinerary)
{
    char query[MAX_QUERY] = {0};
    snprintf(query,MAX_QUERY,"update object set itinerary = itinerary + %d where imei = \'%s\'", itinerary, imeiName);
    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return NULL;
    }

    if(mysql_query(conn, query))
    {
        LOG_FATAL("can't get objects from db(%u, %s)", mysql_errno(conn), mysql_error(conn));
        return NULL;
    }

    LOG_INFO("update Itinerary imei(%s) itnerary(%d)", imeiName, itinerary);
    return 0;
}

static int _db_getItinerary(const char *imeiName)
{
    int itinerary = 0;
    char query[MAX_QUERY] = {0};
    snprintf(query,MAX_QUERY,"select itinerary from object where imei = \'%s\'", imeiName);
    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return NULL;
    }

    if(mysql_query(conn, query))
    {
        LOG_FATAL("can't get objects from db(%u, %s)", mysql_errno(conn), mysql_error(conn));
        return NULL;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;
    result = mysql_store_result(conn);

    if(row = mysql_fetch_row(result))
    {
        itinerary = atoi(row[0]);
    }

    mysql_free_result(result);
    return itinerary;

}

static int _db_getAppPackage(void *action, void *userdata)
{
    GET_APP_PACKAGE_PROC fun = action;
    int itinerary = 0;
    char query[MAX_QUERY] = {0};
    int rc = 0;
    snprintf(query,MAX_QUERY,"select * from AppPackage order by id desc limit 1");
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

    if(row = mysql_fetch_row(result))
    {
        fun(row[1], atoi(row[2]), row[3], row[4], atoi(row[5]), userdata);
    }
    else
    {
        rc = -1;
    }

    mysql_free_result(result);
    return rc;

}


static int _db_getFirmwarePkg(int oldVersion, int *pLastVersion, char *fileName)
{
    int rc = -1;
    char query[MAX_QUERY] = {0};

    int major = oldVersion>>16 & 0xff;
    int minor = oldVersion>>8 & 0xff;
    int micro = oldVersion & 0xff;
    int maxVersion = (major << 16 | (minor + 1) << 8 | 0);

    LOG_INFO("oldVersion: %d, maxVersion: %d", oldVersion, maxVersion);
    snprintf(query,MAX_QUERY,
            "select * from FirmwarePkg where version > %d and version < %d order by version desc limit 1",
            oldVersion, maxVersion);
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

    if(row = mysql_fetch_row(result))
    {
        *pLastVersion = atoi(row[0]);
        strcpy(fileName, row[1]);
        rc = 0;
    }
    else
    {
        rc = -1;
    }

    mysql_free_result(result);
    return rc;
}

static int _db_isMotorCycle(const char *imeiName)
{
    int rc = 0;
    char query[MAX_QUERY] = {0};

    snprintf(query,MAX_QUERY, "select * from motorcycle where imei='%s'", imeiName);
    if(mysql_ping(conn))
    {
        LOG_ERROR("can't ping mysql(%u, %s)",mysql_errno(conn), mysql_error(conn));
        return 0;
    }

    if(mysql_query(conn, query))
    {
        LOG_FATAL("can't get objects from db(%u, %s)", mysql_errno(conn), mysql_error(conn));
        return 0;
    }

    MYSQL_ROW row;
    MYSQL_RES *result;
    result = mysql_store_result(conn);

    if(row = mysql_fetch_row(result))
    {
        rc = 1;
    }

    mysql_free_result(result);
    return rc;
}

int db_initial(void)
{
#ifdef WITH_DB
    return _db_initial();
#else
    return 0;
#endif
}
int db_destruct(void)
{
#ifdef WITH_DB
    return _db_destruct();
#else
    return 0;
#endif
}

int db_isTableCreated(const char* imeiName, int *num)
{
#ifdef WITH_DB
    return _db_isTableCreated(imeiName, num);
#else
    return 0;
#endif
}

int db_createGPS(const char* tableName)
{
#ifdef WITH_DB
    return _db_createGPS(tableName);
#else
    return 0;
#endif
}

int db_getLastGPS(OBJECT *obj)
{
#ifdef WITH_DB
    return _db_getLastGPS(obj);
#else
    return 0;
#endif
}

int db_createCGI(const char* tableName)
{
#ifdef WITH_DB
    return _db_createCGI(tableName);
#else
    return 0;
#endif
}

int db_createiItinerary(const char* tableName)
{
#ifdef WITH_DB
    return _db_createiItinerary(tableName);
#else
    return 0;
#endif
}

int db_saveiItinerary(const char* tableName, int starttime, float startlat, float startlon, int endtime, float endlat, float endlon, int itinerary)
{
#ifdef WITH_DB
    return _db_saveiItinerary(tableName, starttime, startlat, startlon, endtime, endlat, endlon, itinerary);
#else
    return 0;
#endif
}

int db_saveGPS(const char* imeiName, int timestamp, float lat, float lon, char speed, short course)
{
#ifdef WITH_DB
    return _db_saveGPS(imeiName, timestamp, lat, lon, speed, course);
#else
    return 0;
#endif
}

int db_getHistoryGPS(const char *imeiName, int starttime, int endtime, void *action, void *userdata)
{
#ifdef WITH_DB
    return _db_getHistoryGPS(imeiName, starttime, endtime, action, userdata);
#else
    return 0;
#endif
}

int db_getGPS(const char *imeiName, void *action, void *userdata)
{
#ifdef WITH_DB
    return _db_getGPS(imeiName, action, userdata);
#else
    return 0;
#endif
}

int db_getiItinerary(const char *imeiName, int starttime, int endtime, void *action, void *userdata)
{
#ifdef WITH_DB
    return _db_getiItinerary(imeiName, starttime, endtime, action, userdata);
#else
    return 0;
#endif
}

int db_deleteTelNumber(const char *imeiName)
{
#ifdef WITH_DB
    return _db_deleteTelNumber(imeiName);
#else
    return 0;
#endif
}

int db_replaceTelNumber(const char *imeiName, const char *telNumber)
{
#ifdef WITH_DB
    return _db_replaceTelNumber(imeiName, telNumber);
#else
    return 0;
#endif
}

int db_updateCallNumber(const char *imeiName, const char *callNumber)
{
#ifdef WITH_DB
    return _db_updateCallNumber(imeiName, callNumber);
#else
    return 0;
#endif
}

int db_getTelNumber(const char *imeiName, char *telNumber, char *callNumber)
{
#ifdef WITH_DB
    return _db_getTelNumber(imeiName, telNumber, callNumber);
#else
    return 0;
#endif
}

int db_saveCGI(const char* imeiName, int timestamp, const CGI_MC cell[], int cellNo)
{
#ifdef WITH_DB
    return _db_saveCGI(imeiName, timestamp, cell, cellNo);
#else
    return 0;
#endif
}

int db_doWithOBJ(void (*func)(const char*), void (*func2)(const char *), int ObjectType)
{
#ifdef WITH_DB
    return _db_doWithOBJ(func, func2, ObjectType);
#else
    return 0;
#endif
}

int db_insertOBJ(const char *imeiName, int ObjectType, char voltage)
{
#ifdef WITH_DB
    return _db_insertOBJ(imeiName, ObjectType, voltage);
#else
    return 0;
#endif
}

int db_updateSimInfo(const char *imeiName, const char *ccid, const char *imsi)
{
#ifdef WITH_DB
    return _db_updateSimInfo(imeiName, ccid, imsi);
#else
    return 0;
#endif
}

int db_updateOBJIsPosted(const char *imeiName)
{
#ifdef WITH_DB
    return _db_updateOBJIsPosted(imeiName);
#else
    return 0;
#endif
}

int db_ResaveOBJUnpostedImei_cb(void (*func1)(const char*))
{
#ifdef WITH_DB
    return _db_ResaveOBJUnpostedImei_cb(func1);
#else
    return 0;
#endif
}

int db_doWithObjectID(int (*func1)(const char*, const char*))
{
#ifdef WITH_DB
    return _db_doWithObjectID(func1);
#else
    return 0;
#endif
}

int db_add_ObjectID(const char *imei, const char *objectID)
{
#ifdef WITH_DB
    return _db_add_ObjectID(imei, objectID);
#else
    return 0;
#endif
}

int db_add_log(const char *imei, const char *event)
{
#ifdef WITH_DB
    return _db_add_log(imei, event);
#else
    return 0;
#endif
}

int db_updateItinerary(const char *imeiName, long itinerary)
{
#ifdef WITH_DB
    return _db_updateItinerary(imeiName, itinerary);
#else
    return 0;
#endif
}

int db_getItinerary(const char *imeiName)
{
#ifdef WITH_DB
    return _db_getItinerary(imeiName);
#else
    return 0;
#endif
}

int db_getAppPackage(void *action, void *userdata)
{
#ifdef WITH_DB
    return _db_getAppPackage(action, userdata);
#else
    return 0;
#endif
}

int db_getFirmwarePkg(int oldVersion, int *pLastVersion, char *fileName)
{
#ifdef WITH_DB
    return _db_getFirmwarePkg(oldVersion, pLastVersion, fileName);
#else
    return 0;
#endif
}

int db_isMotorCycle(const char *imeiName)
{
#ifdef WITH_DB
    return _db_isMotorCycle(imeiName);
#else
    return 0;
#endif
}


