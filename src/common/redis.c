#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

    redisReply *reply = redisCommand(c,"auth %s", AUTH_REDIS);
    LOG_DEBUG("result: %s", reply->str);

    if(!strstr(reply->str, "OK"))
    {
        LOG_ERROR("Login redis server error");
        return -1;

    }
    freeReplyObject(reply);

    return 0;
}

/*
* func:add device server from redis, if exists, this action will cover it
* param: imei [IN]
*/
int redis_AddDevice(const char *imei)
{
    redisReply *reply;
    char hostSimcom[20] = HOST_SIMCOM;

    reply = redisCommand(c,"PING");
    LOG_DEBUG("PING: %s", reply->str);
    freeReplyObject(reply);

    if(redis_getSelfSimcomIp(hostSimcom) == -1)
    {
        LOG_ERROR("Get Self IP error");
        return -1;
    }
    reply = redisCommand(c,"SET %s %s:%d", imei,hostSimcom, PORT_SIMCOMHTTP);
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
int redis_getSelfSimcomIp(char *hostSimcom)
{
    int sock_num;
    struct ifreq ifR;
    if ( (sock_num = socket(AF_INET, SOCK_DGRAM, 0)) == -1 )
    {
        LOG_ERROR("create socket error:%s\n",strerror(errno));
        return -1;
    }
    strcpy(ifR.ifr_name,"eth0");
    //SIOCGIFADDR Get interface address
    if(ioctl(sock_num, SIOCGIFADDR, &ifR) < 0)
    {
        LOG_ERROR("Get self ip error!");
        return -1;
    }
    hostSimcom = inet_ntoa(((struct sockaddr_in*)&(ifR.ifr_addr))->sin_addr);
    close(sock_num);
    return 0;
}

