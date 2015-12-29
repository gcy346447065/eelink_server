/* 
 * File:   db.h
 * Author: jk
 *
 * Created on June 16, 2015, 9:10 AM
 */

#ifndef DB_H
#define	DB_H

#include "macro.h"

/* database settings */
#define DB_HOST "localhost"
#define DB_USER "eelink"
#define DB_PWD  "eelink"
#define DB_PORT (3306)
#define DB_NAME "gps"
#define MAX_QUERY 400

int db_initial(void);
int db_destruct(void);

int db_isTableCreated(const char* imeiName, int *num);
int db_createGPS(const char* tableName);
int db_createCGI(const char* tableName);
int db_saveGPS(const char* imeiName, int timestamp, float lat, float lon, char speed, short course);
int db_saveCGI(const char* imeiName, int timestamp, const CGI_MC cell[], int cellNo);

int db_doWithOBJ(void (*func)(const char*, int), void (*func2)(const char *));
int db_insertOBJ(const char *imeiName);
int db_updateOBJIsPosted(const char *imeiName);
int db_ResaveOBJUnpostedImei_cb(void (*func1)(const char*));

#endif	/* DB_H */

