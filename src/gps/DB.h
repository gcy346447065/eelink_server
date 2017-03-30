//
// Created by jk on 16-6-25.
//

#ifndef ELECTROMBILE_DB_H
#define ELECTROMBILE_DB_H

#include "mysqlconn_wrapper.h"
#include "setting.h"
#include "protocol.h"

class DB {
private:
    MySQLConnWrapper db_conn;

    static string host;
    static string user;
    static string password;
    static string database;

private:
    DB():db_conn(host, user, password)
    {
        host     = setting.db_host;
        user     = setting.db_user;
        password = setting.db_pwd;
        database = setting.db_database;

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
