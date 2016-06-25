//
// Created by jk on 16-6-25.
//

#include "DB.h"


const string DB::host     = "tcp://127.0.0.1:3306";
const string DB::user     = "eelink";
const string DB::password = "eelink";
const string DB::database = "gps";

void DB::addGPS(string imei, GPS *gps) {
    string sql = "insert into gps_%s(timestamp,lat,lon,speed,course) values(%d,%f,%f,%d,%d)";
    db_conn.prepare(sql);
    db_conn.setString(1, imei);
    db_conn.setInt(2, gps->timestamp);
    //TODO: to be completed

    db_conn.execute(sql);
}
