//
// Created by jk on 16-6-25.
//

#ifndef ELECTROMBILE_DB_H
#define ELECTROMBILE_DB_H

#include "mysqlconn_wrapper.h"
#include "protocol.h"

class DB {
private:
    MySQLConnWrapper db_conn;

    static const string host;
    static const string user;
    static const string password;
    static const string database;

private:
    DB():db_conn(host, user, password)
    {
        db_conn.connect();
        db_conn.switchDb(database);
    }

public:
    static DB& instance()
    {
        static DB instance;
        return instance;
    }

    DB(DB const&)               = delete;
    void operator=(DB const&)   = delete;

public:
    void addGPS(string imei, GPS *gps);
};


#endif //ELECTROMBILE_DB_H
