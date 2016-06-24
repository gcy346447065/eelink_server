/*
 * msg_proc_app.c
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <object.h>

#include "msg_app.h"
#include "msg_tk115.h"
#include "msg_proc_tk115.h"
#include "msg_proc_app.h"
#include "log.h"
#include "macro.h"
#include "session.h"
#include "object.h"
#include "mqtt.h"
#include "cJSON.h"

typedef int (*APP_MSG_PROC)(cJSON*, OBJECT*);

static void app_sendRawData2TK115(const void* msg, int len, const char *imei, int token)
{
	OBJECT* obj = obj_get(imei);
	SESSION *session = obj->session;
	if(!session)
	{
		LOG_ERROR("obj %s offline", imei);
        return;
	}

	MC_MSG_OPERATOR_REQ* req = (MC_MSG_OPERATOR_REQ *)alloc_msg(CMD_OPERAT, sizeof(MC_MSG_OPERATOR_REQ) + len);
	if(req)
	{
		req->type = 0x01;	//FIXME: use enum instead
		req->token = token;
		memcpy(req->data, msg, len);
        msg_send(req, sizeof(MC_MSG_OPERATOR_REQ) + len, session);
	}
	else
	{
		LOG_FATAL("insufficient memory");
	}
}

static int app_sendFenceOnMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    appMsg = appMsg;
    const char data[]="FENCE,1,OR,,,100#";

    LOG_INFO("%s",data);

    app_sendRawData2TK115(data, strlen(data), obj->IMEI, CMD_DEFENCE_ON);
    app_sendCmdRsp2App(APP_CMD_FENCE_ON, CODE_WAITING, obj->IMEI);

    return 0;
}

static int app_sendFenceOffMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    appMsg = appMsg;
    const char data[]="FENCE,0#";

    LOG_INFO("%s",data);

    app_sendRawData2TK115(data, strlen(data), obj->IMEI, CMD_DEFENCE_OFF);
    app_sendCmdRsp2App(APP_CMD_FENCE_OFF, CODE_WAITING, obj->IMEI);

    return 0;
}

static int app_sendFenceGetMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    appMsg = appMsg;
    const char data[]="FENCE,1?";

    LOG_INFO("%s",data);

    app_sendRawData2TK115(data, strlen(data), obj->IMEI, CMD_DEFENCE_GET);
    app_sendCmdRsp2App(APP_CMD_FENCE_GET, CODE_WAITING, obj->IMEI);

    return 0;
}

static int app_sendLocationMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    appMsg = appMsg;
    const char data[]="WHERE#";

    LOG_INFO("location");

    app_sendRawData2TK115(data, strlen(data), obj->IMEI, CMD_LOCATION);
    app_sendCmdRsp2App(APP_CMD_LOCATION, CODE_WAITING, obj->IMEI);

    return 0;
}

static void getImeiFromTopic(const char* topic, char* IMEI)
{
    const char* pStart = &topic[strlen("app2dev/")];
    const char* pEnd = strstr(pStart, "/");

    if (pEnd - pStart > IMEI_LENGTH)
    {
        LOG_ERROR("app2dev: imei length too long");
        return;
    }

    strncpy(IMEI, pStart, pEnd - pStart);

    return;
}

typedef struct
{
    char cmd;
    APP_MSG_PROC pfn;
} APP_MSG_PROC_MAP;

APP_MSG_PROC_MAP msg_proc_map[] =
{
    {APP_CMD_FENCE_ON,          app_sendFenceOnMsg2Device},
    {APP_CMD_FENCE_OFF,         app_sendFenceOffMsg2Device},
    {APP_CMD_FENCE_GET,         app_sendFenceGetMsg2Device},
    {APP_CMD_LOCATION,          app_sendLocationMsg2Device},
};

int app_handleApp2devMsg(const char* topic, const char* data, const int len __attribute__((unused)))
{
    if (!data)
    {
        LOG_FATAL("internal error: payload null");
        return -1;
    }

    LOG_DEBUG("topic = %s, payload = %s", topic, data);

    /* get imei and object from topic */
    char strIMEI[IMEI_LENGTH + 1] = {0};
    getImeiFromTopic(topic, strIMEI);
    OBJECT* obj = obj_get(strIMEI);
    if (!obj)
    {
        LOG_ERROR("obj %s not exist", strIMEI);
        return -1;
    }

    /* get appMsg and cmd from data */
    cJSON* appMsg = cJSON_Parse(data);
    if (!appMsg)
    {
        LOG_ERROR("app message format not json: %s", cJSON_GetErrorPtr());
        return -1;
    }
    cJSON* cmdItem = cJSON_GetObjectItem(appMsg, "cmd");
    if (!cmdItem)
    {
        LOG_ERROR("no cmd item");
        return -1;
    }
    int cmd = cmdItem->valueint;

    /* if offline, send CODE_DEVICE_OFFLINE;
     * if offline with APP_CMD_LOCATION, send CODE_DEVICE_OFFLINE with GPS;
     */
    if(!(obj->session))
    {
        LOG_WARN("simcom %s offline", strIMEI);

        if(cmd != APP_CMD_LOCATION)
        {
            app_sendCmdRsp2App(cmd, CODE_DEVICE_OFFLINE, strIMEI);
        }
        else
        {
            app_sendLocationRsp2App(CODE_DEVICE_OFFLINE, obj);
        }

        return 0;
    }

    LOG_INFO("receive app cmd:%d", cmd);

    for (size_t i = 0; i < sizeof(msg_proc_map) / sizeof(msg_proc_map[0]); i++)
    {
        if (msg_proc_map[i].cmd == cmd)
        {
            APP_MSG_PROC pfn = msg_proc_map[i].pfn;
            if (pfn)
            {
                return pfn(appMsg, obj);
            }
        }
    }

    LOG_ERROR("unknown app cmd:%d", cmd);

    return -1;
}


