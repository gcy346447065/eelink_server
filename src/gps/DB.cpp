//
// Created by jk on 16-6-25.
//

#include "DB.h"
#include "protocol.h"


const string DB::host     = "tcp://127.0.0.1:3306";
const string DB::user     = "eelink";
const string DB::password = "eelink";
const string DB::database = "gps";

void DB::addGPS(string imei, GPS *gps)
{
    string sql = "insert into `gps_" + imei + "`(`timestamp`,`lat`,`lon`,`speed`,`course`) values(?,?,?,?,?)";

    db_conn.prepare(sql);
    db_conn.setInt(1, gps->timestamp);
    db_conn.setFloat(2, gps->latitude);
    db_conn.setFloat(3, gps->longitude);
    db_conn.setInt(4, gps->speed);
    db_conn.setInt(5, gps->course);

    db_conn.execute();
}
