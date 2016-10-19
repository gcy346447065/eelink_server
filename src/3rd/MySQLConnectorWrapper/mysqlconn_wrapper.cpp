/*
 * File:   mysqlconn_wrapper.h
 * Author: Eduardo Casas (www.eduardocasas.com)
 *
 * Created on February 24, 2013, 5:07 PM
 *
 * This is a wrapper class for the MySQL Connector/C++ Library
 * see: http://dev.mysql.com/doc/connector-cpp/en/index.html
 */

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "mysqlconn_wrapper.h"

using namespace std;

MySQLConnWrapper::~MySQLConnWrapper()
{
    delete res;
    delete prep_stmt;
    delete stmt;
    delete con;
}

void MySQLConnWrapper::manageException(sql::SQLException& e, string file, int line, string function)
{
    if (e.getErrorCode() != 0){
        cout << "# ERR: SQLException in " << file;
        cout << "(" << function << ") on line " << line << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }
}

void MySQLConnWrapper::connect()
{
    try{
        driver = get_driver_instance();
        con = driver->connect(host, user, password);
    } catch (sql::SQLException &e){
        manageException(e, __FILE__, __LINE__, __FUNCTION__);
    }
}

void MySQLConnWrapper::switchDb(const string& db_name)
{
    try{
        con->setSchema(db_name);
        stmt = con->createStatement();
    } catch (sql::SQLException &e) {
        manageException(e, __FILE__, __LINE__, __FUNCTION__);
    }
}

void MySQLConnWrapper::prepare(const string& query)
{
    try{
        prep_stmt = con->prepareStatement(query);
    } catch (sql::SQLException &e){
        manageException(e, __FILE__, __LINE__, __FUNCTION__);
    }
}

void MySQLConnWrapper::setInt(const int& num, const int& data)
{
    prep_stmt->setInt(num, data);
}

void MySQLConnWrapper::setString(const int& num, const string& data)
{
    prep_stmt->setString(num, data);
}

void MySQLConnWrapper::setFloat(const int &num, const float &data)
{
    prep_stmt->setDouble(num, data);
}
void MySQLConnWrapper::execute(const string& query)
{

    try {
    	if (query != "") {
    		res = stmt->executeQuery(query);
    	} else {
    		res = prep_stmt->executeQuery();
    		prep_stmt->close();
    	}
    } catch (sql::SQLException &e) {
        manageException(e, __FILE__, __LINE__, __FUNCTION__);
    }
}

bool MySQLConnWrapper::fetch()
{
    return res->next();
}

string MySQLConnWrapper::print(const string& field)
{
    return res->getString(field);
}

string MySQLConnWrapper::print(const int& index)
{
    return res->getString(index);
}
