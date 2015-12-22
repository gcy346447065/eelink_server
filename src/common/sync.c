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

static void sendMsg2Sync(void* data, size_t len)
{
    bufferevent_write(bev, data, len);
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
    //sin.sin_port = htons(PORT_SYNC);
    sin.sin_port = htons(8889);

    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    if (bufferevent_socket_connect(bev,
        (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        LOG_ERROR("cannot connect to the sync server");
        bufferevent_free(bev);
        bev = NULL;

        //TODO: start a timer to re-connect to the sync server
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
        //TODO: log error
        cJSON_Delete(root);
        return;
    }

    sendMsg2Sync(data, strlen(data));
    LOG_INFO("sync %s to leancloud", imei);

    free(data);
    cJSON_Delete(root);

    return;
}

void sync_gps(const char* imei, float lat, float lng)
{
    cJSON* root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, TAG_CMD, CMD_SYNC_NEW_GPS);
    cJSON_AddStringToObject(root, TAG_IMEI, imei);
    cJSON_AddNumberToObject(root, TAG_LAT, lat);
    cJSON_AddNumberToObject(root, TAG_LNG, lng);

    char *data = cJSON_PrintUnformatted(root);
    if (!data)
    {
        //TODO: log error
        cJSON_Delete(root);
        return;
    }

    sendMsg2Sync(data, strlen(data));

    free(data);
    cJSON_Delete(root);

    return;
}

