//
// Created by lc on 17-03-25.
//

#ifndef __USER_SETTING_H__
#define __USER_SETTING_H__

#define MAX_DOMAIN_LEN (64)
#define MAX_NAME_LEN (32)
typedef struct{
    struct
    {
        char simcom_host[MAX_DOMAIN_LEN];
        int  simcom_port;
        int  simhttp_port;
        int  tk115_port;
        int  sync_port;
    };
    struct
    {
        char mqtt_host[MAX_DOMAIN_LEN];
        int  mqtt_port;
    };
    struct
    {
        char db_host[MAX_DOMAIN_LEN];
        int  db_port;
        char db_user[MAX_NAME_LEN];
        char db_pwd[MAX_NAME_LEN];
        char db_database[MAX_NAME_LEN];
    };
    struct
    {
        char http_host[MAX_DOMAIN_LEN];
        int  http_port;
    };
    struct
    {
        char redis_host[MAX_DOMAIN_LEN];
        int  redis_port;
        int  redis_auth[MAX_NAME_LEN];
    };
} SETTING;
extern SETTING setting;

int setting_initail(char *file);

#endif/*__USER_SETTING_H__*/
