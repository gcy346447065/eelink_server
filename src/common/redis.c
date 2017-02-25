#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include "redis.h"
#include "log.h"
#include "port.h"

static redisContext *c = NULL;

/*
* func:connect redis server
* return: if success return SUCCESS, else return -1
*/
int redis_initial(void)
{
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    c = redisConnectWithTimeout(HOST_REDIS, PORT_REDIS, timeout);
    if (!c) {
        LOG_FATAL("redis connection error: can't allocate redis context");
        return -1;
    }
    else if(c->err) {
        LOG_FATAL("redis connection error: %s", c->errstr);
        redisFree(c);
        return -1;
    }

    return 0;
}

/*
* func:add device server from redis, if exists, this action will cover it
* param: imei [IN]
*/
int redis_AddDevice(const char *imei)
{
    redisReply *reply;
    char *hostSimcon = HOST_SIMCOM;

    reply = redisCommand(c,"PING");
    LOG_DEBUG("PING: %s", reply->str);
    freeReplyObject(reply);

    if(redis_getSelfSimcomIp(hostSimcon) == -1)
    {
        LOG_ERROR("Get Self IP error");
    }
    reply = redisCommand(c,"SET %s %s:%d", imei,hostSimcon, PORT_SIMCOMHTTP);
    LOG_INFO("SET: %s %s", imei, reply->str);
    freeReplyObject(reply);

    return 0;
}

/*
* func:delete device server from redis
* param: imei [IN]
*/
int redis_DelDevice(const char *imei)
{
    redisReply *reply;

    reply = redisCommand(c,"PING");
    LOG_DEBUG("PING: %s", reply->str);
    freeReplyObject(reply);

    reply = redisCommand(c,"DEL %s", imei);
    LOG_INFO("DEL: %s %s", imei, reply->str);
    freeReplyObject(reply);

    return 0;
}

/*
* func:get device server from redis
* param: imei [IN], hostNamewithPort [OUT]
* return: if exist return SUCCESS, else return -1
*/
int redis_getDeviceServer(const char *imei, char *hostNamewithPort)
{
    redisReply *reply;

    reply = redisCommand(c,"PING");
    LOG_DEBUG("PING: %s", reply->str);
    freeReplyObject(reply);

    reply = redisCommand(c,"GET %s", imei);
    if(NULL == reply->str)
    {
        LOG_INFO("Device %s if offline", imei);
        freeReplyObject(reply);
        return -1;
    }

    LOG_INFO("GET: %s %s", imei, reply->str);
    strncpy(hostNamewithPort, reply->str, MAX_HOSTNAMEWITHPORT_LEN);
    freeReplyObject(reply);

    return 0;
}

/*
* func:get device server self IP from redis
* param: hostSimcom [IN][OUT]
* return: if get IP return SUCCESS, else return -1
*/
int redis_getSelfSimcomIp(const char *hostSimcom)
{
    struct hostent *host;
    char hostname[20] = {0};
    if(gethostname(hostname,sizeof(hostname)) < 0)
    {
        LOG_ERROR("Get self IP error in the redis_getSelfSimcomIp function first step");
        return -1;
    }
    if((host = gethostbyname(hostname) == NULL)
    {
        LOG_ERROR("Get self IP error in the redis_getSelfSimcomIp function second step");
        return -1;
    }
    hostSimcom = inet_ntoa(*(struct in_addr*)(host->h_addr));
    return 0;
}

