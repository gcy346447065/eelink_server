/*
 * cell.c
 *
 *  Created on: Oct 1, 2015
 *      Author: jk
 */


#include <event2/event.h>
#include <event2/bufferevent.h>
#include <string.h>
#include <malloc.h>

#include "sync.h"
#include "cJSON.h"
#include "inter_msg.h"

static struct bufferevent *bev = NULL;

static void eventcb(struct bufferevent *bev, short events, void *ptr)
{
    if (events & BEV_EVENT_CONNECTED)
    {
         /* We're connected to the server.   Ordinarily we'd do
            something here, like start reading or writing. */
    	//TODO:
    }
    else if (events & BEV_EVENT_ERROR)
    {
    	//TODO
    }
}

static void read_cb(struct bufferevent *bev, void *ctx)
{
	char buf[1024] = {0};
	size_t n = 0;

    while ((n = bufferevent_read(bev, buf, sizeof(buf))) > 0)
    {
    	//TODO
    }
}

static void write_cb(struct bufferevent* bev, void *ctx)
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
    sin.sin_port = htons(40713); /* Port 8080 */

    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, read_cb, write_cb, eventcb, NULL);
    bufferevent_enable(bev, EV_READ|EV_WRITE);


    if (bufferevent_socket_connect(bev,
        (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        /* Error starting connection */
        bufferevent_free(bev);
        return -1;
    }

    return 0;
}

int sync_exit()
{
    //TODO: close the socket when exit
    return 0;
}

void sync_newIMEI(const char *imei)
{

    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, TAG_CMD, CMD_SYNC_NEW_DID);
    cJSON_AddStringToObject(root, TAG_IMEI, imei);

    char *data = cJSON_PrintUnformatted(root);
    if (!data) {
        //TODO: log error
        return;
    }

    bufferevent_write(bev, data, strlen(data));

    free(data);
    cJSON_Delete(root);

    return;
}



