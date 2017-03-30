//
// Created by lc on 17-03-25.
//
#include <iniparser.h>

#include "log.h"
#include "setting.h"

SETTING setting;
int setting_initail(char *file)
{
    dictionary *ini = iniparser_load(file);//parser the file
    if(!ini)
    {
        LOG_FATAL("can not open %s!", file);
        return -1;
    }


    /* simcom server */
    char *simcom_host = iniparser_getstring(ini,"simcom:host", NULL);
    if(!simcom_host)
    {
        LOG_FATAL("simcom host dose not exist");
        return -1;
    }
    strncpy(setting.simcom_host, simcom_host, MAX_DOMAIN_LEN);

    int simcom_port = iniparser_getint(ini,"simcom:simcom_port", 0);
    setting.simcom_port = simcom_port?simcom_port:9880;

    int tk115_port = iniparser_getint(ini,"simcom:tk115_port", 0);
    setting.tk115_port = tk115_port?tk115_port:9870;

    int sync_port = iniparser_getint(ini,"simcom:sync_port", 0);
    setting.sync_port = sync_port?sync_port:9890;

    int simhttp_port = iniparser_getint(ini,"simcom:http_port", 0);
    setting.simhttp_port = simhttp_port?simhttp_port:8082;


    /* mqtt server */
    char *mqtt_host = iniparser_getstring(ini,"mqtt:host", NULL);
    if(!mqtt_host)
    {
        LOG_FATAL("mqtt host dose not exist");
        return -1;
    }
    strncpy(setting.mqtt_host, mqtt_host, MAX_DOMAIN_LEN);

    int mqtt_port = iniparser_getint(ini,"mqtt:port",-1);
    setting.mqtt_port = mqtt_port?mqtt_port:1883;


    /* db server */
    char *db_host = iniparser_getstring(ini,"db:host", NULL);
    if(!db_host)
    {
        LOG_FATAL("db host dose not exist");
        return -1;
    }
    strncpy(setting.db_host, db_host, MAX_DOMAIN_LEN);


    int db_port = iniparser_getint(ini,"db:port", 0);
    setting.db_port = db_port?db_port:3306;

    char *db_user = iniparser_getstring(ini,"db:user", NULL);
    if(!db_user)
    {
        LOG_FATAL("db user dose not exist");
        return -1;
    }
    strncpy(setting.db_user, db_user, MAX_NAME_LEN);

    char *db_pwd = iniparser_getstring(ini,"db:password","null");
    if(!db_pwd)
    {
        LOG_FATAL("db password dose not exist");
        return -1;
    }
    strncpy(setting.db_pwd, db_pwd, MAX_NAME_LEN);

    char *db_database = iniparser_getstring(ini,"db:database", NULL);
    if(!db_database)
    {
        LOG_FATAL("db database dose not exist");
        return -1;
    }
    strncpy(setting.db_database, db_database, MAX_NAME_LEN);


    /* redis server */
    char *redis_host = iniparser_getstring(ini,"redis:host", NULL);
    if(!db_host)
    {
        LOG_FATAL("redis host dose not exist");
        return -1;
    }
    strncpy(setting.redis_host, redis_host, MAX_DOMAIN_LEN);

    int redis_port = iniparser_getint(ini,"redis:port", 0);
    setting.redis_port = redis_port?redis_port:6379;

    char *redis_auth = iniparser_getstring(ini,"redis:auth", NULL);
    if(!db_host)
    {
        LOG_FATAL("redis auth dose not exist");
        return -1;
    }
    strncpy(setting.redis_auth, redis_auth, MAX_NAME_LEN);


    iniparser_freedict(ini);//free dirctionary obj
    return 0;
}

