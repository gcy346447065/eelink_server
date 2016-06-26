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

void DB::addGPS(char *imei, GPS *gps)
{
    char Csql[256] = {0};

    /*it's not OK,dont know why*/
    /*
    snprintf(test,128,"insert into gps_%s(timestamp,lat,lon,speed,course) values(%d,%f,%f,%d,%d)",\
                        imei,gps->timestamp,gps->latitude,gps->longitude,gps->speed,gps->course);
    */

    /*it's just OK,dont know why*/
    snprintf(Csql,128,"insert into gps_865067021652600(timestamp,lat,lon,speed,course) values(%d,%f,%f,%d,%d)",\
                        gps->timestamp,gps->latitude,gps->longitude,gps->speed,gps->course);

    string sql = Csql;
    printf("%s\r\n",sql.data());
    db_conn.prepare(sql);

    db_conn.execute(sql);
}
