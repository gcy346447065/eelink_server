//
// Created by jk on 16-6-25.
//
#include <netinet/in.h>

#include "DB.h"
#include "logger.h"
#include "protocol.h"

string DB::host = "127.0.0.1";
string DB::user = "user";
string DB::password = "password";
string DB::database = "database";


void DB::addGPS(string imei, GPS *gps)
{
    string sql = "insert into `gps_" + imei + "`(`timestamp`,`lat`,`lon`,`speed`,`course`) values(?,?,?,?,?)";

    db_conn.prepare(sql);
    db_conn.setInt(1, ntohl(gps->timestamp));
    db_conn.setFloat(2, gps->latitude);
    db_conn.setFloat(3, gps->longitude);
    db_conn.setInt(4, gps->speed);
    db_conn.setInt(5, ntohs(gps->course));

    db_conn.execute();
}
