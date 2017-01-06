/*
 * msg_proc_http.c
 *
 *  Created on: 2016/10/13
 *      Author: lc
 */
#include <string.h>
#include "msg_proc_http.h"

#include "gps_http.h"
#include "device_http.h"
#include "package_http.h"
#include "itinerary_http.h"
#include "telephone_http.h"

#define MAX_CMD_LENGTH 32

typedef void (*MSG_PROC)(struct evhttp_request *req, struct event_base *base);
typedef struct
{
	const char cmd[MAX_CMD_LENGTH];
	MSG_PROC pfn;
} MSG_PROC_MAP;

static MSG_PROC_MAP msgProcs[] =
{
	{"/v1/history",     http_replyGPS},
    {"/v1/itinerary",   http_replyItinerary},
	{"/v1/telephone",   http_replyTelephone},
	{"/v1/test",        http_replyCall},
	{"/v1/device",      http_deviceHandler},
	{"/v1/package",     http_replyPackage},
    {"/v1/version",     http_replyVersion},
};

void httpd_handler(struct evhttp_request *req, void *arg)
{
    for(size_t i = 0; i < sizeof(msgProcs)/sizeof(msgProcs[0]); i++)
    {
        if(strstr(req->uri, msgProcs[i].cmd))
        {
            MSG_PROC pfn = msgProcs[i].pfn;
            if (pfn)
            {
                pfn(req, arg);
                return;
            }
        }
    }
}

