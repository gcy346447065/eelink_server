//
// Created by jk on 16-6-25.
//
#include <stdio.h>
#include <netinet/in.h>

#include "DB.h"


const string DB::host     = "tcp://127.0.0.1:3306";
const string DB::user     = "eelink";
const string DB::password = "eelink";
const string DB::database = "gps";

void DB::addGPS(string imei, GPS gps[])
{
/*  string sql = "insert into gps_%s(timestamp,lat,lon,speed,course) values(%d,%f,%f,%d,%d)";
    db_conn.prepare(sql);
    db_conn.setString(1, imei);
    db_conn.setInt(2, gps->timestamp);
*/
    //TODO: to be completed
    char c_sql[128] = {0};
    snprintf(c_sql,128,"insert into gps_%s(timestamp,lat,lon,speed,course) values(%d,%f,%f,%d,%d)",\
                      imei.c_str(),gps->timestamp,gps->latitude,gps->longitude,gps->speed,gps->course);
    string sql = c_sql;
    printf("%s\r\n",sql.c_str());
    //string sql = "insert into gps_865067021652600(timestamp,lat,lon,speed,course) values(1466904520,30.114000,114.005234,0,0)";
    db_conn.execute(sql);
}
