#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    reply = redisCommand(c,"PING");
    LOG_DEBUG("PING: %s", reply->str);
    freeReplyObject(reply);

    reply = redisCommand(c,"SET %s %s:%d", imei, HOST_SIMCOM, PORT_SIMCOMHTTP);
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

