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
        LOG_FATAL("can not open %s!","eelink_server.ini");
        return -1;
    }


    /* simcom server */
    char *simcom_host = iniparser_getstring(ini,"simcom:host","null");
    if(!simcom_host)
    {
        LOG_FATAL("simcom host dose not exist");
        return -1;
    }
    strncpy(setting.simcom_host, simcom_host, MAX_DOMAIN_LEN);

    int simcom_port = iniparser_getint(ini,"simcom:simcom_port",-1);
    if(!simcom_port)
    {
        LOG_FATAL("simcom port dose not exist");
        return -1;
    }
    setting.simcom_port = simcom_port;

    int tk115_port = iniparser_getint(ini,"simcom:tk115_port",-1);
    if(!tk115_port)
    {
        LOG_FATAL("simcom port dose not exist");
        return -1;
    }
    setting.tk115_port = tk115_port;

    int sync_port = iniparser_getint(ini,"simcom:sync_port",-1);
    if(!sync_port)
    {
        LOG_FATAL("simcom port dose not exist");
        return -1;
    }
    setting.sync_port = sync_port;

    int simhttp_port = iniparser_getint(ini,"simcom:http_port",-1);
    if(!simhttp_port)
    {
        LOG_FATAL("simcom port dose not exist");
        return -1;
    }
    setting.simhttp_port = simhttp_port;


    /* mqtt server */
    char *mqtt_host = iniparser_getstring(ini,"mqtt:host","null");
    if(!mqtt_host)
    {
        LOG_FATAL("simcom host dose not exist");
        return -1;
    }
    strncpy(setting.mqtt_host, mqtt_host, MAX_DOMAIN_LEN);

    int mqtt_port = iniparser_getint(ini,"mqtt:port",-1);
    if(!mqtt_port)
    {
        LOG_FATAL("simcom port dose not exist");
        return -1;
    }
    setting.mqtt_port = mqtt_port;


    /* db server */
    char *db_host = iniparser_getstring(ini,"db:host","null");
    if(!db_host)
    {
        LOG_FATAL("simcom host dose not exist");
        return -1;
    }
    strncpy(setting.db_host, db_host, MAX_DOMAIN_LEN);


    int db_port = iniparser_getint(ini,"db:port",-1);
    if(!db_port)
    {
        LOG_FATAL("simcom port dose not exist");
        return -1;
    }
    setting.db_port = db_port;

    char *db_user = iniparser_getstring(ini,"db:user","null");
    if(!db_user)
    {
        LOG_FATAL("simcom host dose not exist");
        return -1;
    }
    strncpy(setting.db_user, db_user, MAX_NAME_LEN);

    char *db_pwd = iniparser_getstring(ini,"db:password","null");
    if(!db_pwd)
    {
        LOG_FATAL("simcom host dose not exist");
        return -1;
    }
    strncpy(setting.db_pwd, db_pwd, MAX_NAME_LEN);

    char *db_database = iniparser_getstring(ini,"db:database","null");
    if(!db_database)
    {
        LOG_FATAL("simcom host dose not exist");
        return -1;
    }
    strncpy(setting.db_database, db_database, MAX_NAME_LEN);


    /* redis server */
    char *redis_host = iniparser_getstring(ini,"redis:host","null");
    if(!db_host)
    {
        LOG_FATAL("redis_host dose not exist");
        return -1;
    }
    strncpy(setting.redis_host, redis_host, MAX_DOMAIN_LEN);

    int redis_port = iniparser_getint(ini,"redis:port",-1);
    if(!redis_port)
    {
        LOG_FATAL("redis_port dose not exist");
        return -1;
    }
    setting.redis_port = redis_port;

    char *redis_auth = iniparser_getstring(ini,"redis:auth","null");
    if(!db_host)
    {
        LOG_FATAL("redis_host dose not exist");
        return -1;
    }
    strncpy(setting.redis_auth, redis_auth, MAX_NAME_LEN);
    LOG_INFO("%s:%d %s", setting.redis_host, setting.redis_port, setting.redis_auth);


    iniparser_freedict(ini);//free dirctionary obj
    return 0;
}

