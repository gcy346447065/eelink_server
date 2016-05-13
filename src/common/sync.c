/*
 * cell.c
 *
 *  Created on: Oct 1, 2015
 *      Author: jk
 */


#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>

#include <string.h>
#include <malloc.h>
#include <errno.h>

#include "sync.h"
#include "cJSON.h"
#include "inter_msg.h"
#include "log.h"
#include "port.h"
#include "timer.h"

static struct bufferevent *bev = NULL;
static struct event *evTimerReconnect = NULL;

void sync_reconnect_fn(evutil_socket_t fd, short a, void * arg)
{
    LOG_INFO("re-connect to sync server");
    struct event_base *base = arg;

    sync_init(base);
}

static void sendMsg2Sync(void *data, size_t len)
{
    if(bev)
    {
        bufferevent_write(bev, data, len);
    }
    else
    {
        LOG_INFO("sync server connection dev null, wait to reconnect");
    }
    
    return;
}

static void event_cb(struct bufferevent *bev, short events, void *arg)
{
    struct event_base *base = bev->ev_base;
    if (events & BEV_EVENT_CONNECTED)
    {
         /* We're connected to the server.   Ordinarily we'd do
            something here, like start reading or writing. */
        LOG_INFO("connect to the sync server successful");
        if (evTimerReconnect)
        {
            evtimer_del(evTimerReconnect);
        }
    }
    else if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF))
    {
        if (events & BEV_EVENT_ERROR)
        {
            int err = bufferevent_socket_get_dns_error(bev);
            if (err)
            {
                LOG_ERROR("DNS error: %s\n", evutil_gai_strerror(err));
            }
            LOG_ERROR("connect to sync server error:%s", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        }

        if (events & BEV_EVENT_EOF)
        {
            LOG_INFO("disconneted to sync server");
        }

        struct timeval five_minitue = { 300, 0 };
        if (evTimerReconnect)
        {
            timer_react(evTimerReconnect, &five_minitue);
        }
        else
        {
            evTimerReconnect = timer_newOnce(base, &five_minitue, sync_reconnect_fn, base);
        }

        bufferevent_free(bev);
        bev = NULL;
    }
}

static void read_cb(struct bufferevent *bev, void *arg)
{
	char buf[1024] = {0};
	size_t n = 0;

    while ((n = bufferevent_read(bev, buf, sizeof(buf))) > 0)
    {
    	//TODO
    }
}

static void write_cb(struct bufferevent* bev, void *arg)
{
	//TODO
	return;
}

int sync_init(struct event_base *base)
{
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1 */
    sin.sin_port = htons(PORT_SYNC);

    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    if (bufferevent_socket_connect(bev,
        (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        LOG_ERROR("cannot connect to the sync server");
        bufferevent_free(bev);
        bev = NULL;

        struct timeval five_minitue = { 300, 0 };
        if (evTimerReconnect)
        {
            timer_react(evTimerReconnect, &five_minitue);
        }
        else
        {
            //The event_self_cbarg() function was introduced in Libevent 2.1.1-alpha.
            //evTimerReconnect = timer_newOnce(base, &one_minitue, sync_reconnect_fn, event_self_cbarg());

            evTimerReconnect = timer_newOnce(base, &five_minitue, sync_reconnect_fn, base);
        }
        return -1;
    }
    else
    {
        LOG_DEBUG("connect to sync server...");
    }

    return 0;
}

int sync_exit()
{
    if (evTimerReconnect)
    {
        evtimer_del(evTimerReconnect);
        event_free(evTimerReconnect);
    }

    return 0;
}

void sync_newIMEI(const char *imei)
{
    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, TAG_CMD, CMD_SYNC_NEW_DID);
    cJSON_AddStringToObject(root, TAG_IMEI, imei);

    char *data = cJSON_PrintUnformatted(root);

    if (!data)
    {
        LOG_ERROR("internal error");
        cJSON_Delete(root);
        return;
    }

    sendMsg2Sync(data, strlen(data));
    LOG_INFO("send imei(%s) to sync", imei);

    free(data);
    cJSON_Delete(root);

    return;
}

void sync_gps(const char* imei, int timestamp, float lat, float lng, char speed, short course)
{
    cJSON* root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, TAG_CMD, CMD_SYNC_NEW_GPS);
    cJSON_AddNumberToObject(root, TAG_TIMESTAMP, timestamp);
    cJSON_AddStringToObject(root, TAG_IMEI, imei);
    cJSON_AddNumberToObject(root, TAG_LAT, lat);
    cJSON_AddNumberToObject(root, TAG_LNG, lng);
    cJSON_AddNumberToObject(root, TAG_SPEED, speed);
    cJSON_AddNumberToObject(root, TAG_COURSE, course);

    char *data = cJSON_PrintUnformatted(root);
    if (!data)
    {
        LOG_ERROR("internal error");
        cJSON_Delete(root);
        return;
    }

    sendMsg2Sync(data, strlen(data));
    LOG_INFO("send gps(timestamp(%d), imei(%s), lat(%f), lng(%f), speed(%d), course(%d)) to sync",
            timestamp, imei, lat, lng, speed, course);

    free(data);
    cJSON_Delete(root);
    return;
}

void sync_itinerary(const char *imei, int start, int end, int miles)
{
    cJSON* root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, TAG_CMD, CMD_SYNC_NEW_ITINERARY);
    cJSON_AddStringToObject(root, TAG_IMEI, imei);
    cJSON_AddNumberToObject(root, TAG_START, start);
    cJSON_AddNumberToObject(root, TAG_END, end);
    cJSON_AddNumberToObject(root, TAG_MILES, miles);

    char *data = cJSON_PrintUnformatted(root);
    if (!data)
    {
        LOG_ERROR("internal error");
        cJSON_Delete(root);
        return;
    }

    sendMsg2Sync(data, strlen(data));
    LOG_INFO("send itinerary(start(%d), end(%d), miles(%d) to sync", start, end, miles);

    free(data);
    cJSON_Delete(root);
    return;
}

void sync_SimInfo(const char* imei, const char* ccid, const char* imsi)
{
    cJSON* root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, TAG_CMD, CMD_SYNC_NEW_SIM_INFO);
    cJSON_AddStringToObject(root, TAG_IMEI, imei);
    cJSON_AddStringToObject(root, TAG_CCID, ccid);
    cJSON_AddStringToObject(root, TAG_IMSI, imsi);

    char *data = cJSON_PrintUnformatted(root);
    if (!data)
    {
        LOG_ERROR("internal error");
        cJSON_Delete(root);
        return;
    }

    sendMsg2Sync(data, strlen(data));
    LOG_INFO("send sim_info(imei(%s), ccid(%s), imsi(%s) to sync", imei, ccid, imsi);

    free(data);
    cJSON_Delete(root);

    return;
}
