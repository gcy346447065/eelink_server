/*
 * msg_proc_app.c
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msg_app.h"
#include "msg_simcom.h"
#include "log.h"
#include "session.h"
#include "object.h"
#include "cJSON.h"

typedef int (*APP_MSG_PROC)(cJSON*, OBJECT*);

static void app_sendMsg2Device(void *msg, size_t len, OBJECT *obj)
{
    SESSION *session = obj->session;
    if(!session)
    {
        LOG_ERROR("obj %s offline", obj->IMEI);
        return;
    }

    MSG_SEND pfn = session->pSendMsg;
    if(!pfn)
    {
        LOG_ERROR("device offline");
        return;
    }

    pfn(session->bev, msg, len);

    LOG_DEBUG("send msg(cmd=%d), length(%ld)", get_msg_cmd(msg), len);
    LOG_HEX(msg, len);

    free_simcom_msg(msg);

    return;
}

static char defendApp2mc(int cmd)
{
    if(cmd == APP_CMD_FENCE_ON)
    {
        return DEFEND_ON;
    }
    else if(cmd == APP_CMD_FENCE_OFF)
    {
        return DEFEND_OFF;
    }
    else if(cmd == APP_CMD_FENCE_GET)
    {
        return DEFEND_GET;
    }
    else
    {
        return -1;
    }
}

static char seekApp2mc(int cmd)
{
    if(cmd == APP_CMD_SEEK_ON)
    {
        return SEEK_ON;
    }
    else if(cmd == APP_CMD_SEEK_OFF)
    {
        return SEEK_OFF;
    }
    else
    {
        return -1;
    }
}

static char autolockSetApp2mc(int cmd)
{
    if(cmd == APP_CMD_AUTOLOCK_ON)
    {
        return AUTOLOCK_ON;
    }
    else if(cmd == APP_CMD_AUTOLOCK_OFF)
    {
        return AUTOLOCK_OFF;
    }
    else
    {
        return -1;
    }
}

static int getMsgCmd(cJSON* appMsg)
{
    cJSON* cmdItem = cJSON_GetObjectItem(appMsg, "cmd");
    if(!cmdItem)
    {
        LOG_ERROR("command format error:no cmd item");
        return -1;
    }

    return cmdItem->valueint;
}

static int app_sendWildMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    cJSON *dataItem = cJSON_GetObjectItem(appMsg, "data");
    if(!dataItem) 
    {
        LOG_ERROR("wild cmd with no data");
        return -1;
    }
    char* data = dataItem->string;

    void *msg = alloc_simcomWildMsg(data, strlen(data));
    if(!msg)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(APP_CMD_WILD, CODE_INTERNAL_ERR, obj->IMEI);
        return -1;
    }

    app_sendMsg2Device(msg, MSG_HEADER_LEN + strlen(data), obj);
    return 0;
}

static int app_sendFenceOnMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    appMsg = appMsg;

    MSG_HEADER *req = (MSG_HEADER *)alloc_simcom_msg(CMD_DEFEND_ON, sizeof(MSG_HEADER));
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(APP_CMD_FENCE_ON, CODE_INTERNAL_ERR, obj->IMEI);
        return -1;
    }

    app_sendCmdRsp2App(APP_CMD_FENCE_ON, CODE_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_HEADER), obj);
    return 0;
}

static int app_sendFenceOffMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    appMsg = appMsg;

    MSG_HEADER *req = (MSG_HEADER *)alloc_simcom_msg(CMD_DEFEND_OFF, sizeof(MSG_HEADER));
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(APP_CMD_FENCE_OFF, CODE_INTERNAL_ERR, obj->IMEI);
        return -1;
    }

    app_sendCmdRsp2App(APP_CMD_FENCE_OFF, CODE_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_HEADER), obj);
    return 0;
}

static int app_sendFenceGetMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    appMsg = appMsg;

    MSG_HEADER *req = (MSG_HEADER *)alloc_simcom_msg(CMD_DEFEND_GET, sizeof(MSG_HEADER));
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(APP_CMD_FENCE_GET, CODE_INTERNAL_ERR, obj->IMEI);
        return -1;
    }

    app_sendCmdRsp2App(APP_CMD_FENCE_GET, CODE_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_HEADER), obj);
    return 0;
}

static int app_sendSeekMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    int cmd = getMsgCmd(appMsg);

    MSG_SEEK_REQ *req = (MSG_SEEK_REQ *)alloc_simcomSeekReq(cmd, seekApp2mc(cmd));
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(cmd, CODE_INTERNAL_ERR, obj->IMEI);
        return -1;
    }

    app_sendCmdRsp2App(cmd, CODE_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_SEEK_REQ), obj);
    return 0;
}

static int app_sendLocationMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    appMsg = appMsg;

    MSG_HEADER *req = (MSG_HEADER *)alloc_simcom_msg(CMD_LOCATE, sizeof(MSG_HEADER));
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendLocationRsp2App(CODE_INTERNAL_ERR, obj);
        return -1;
    }

    app_sendLocationRsp2App(CODE_WAITING, obj);
    app_sendMsg2Device(req, sizeof(MSG_HEADER), obj);
    return 0;
}

static int app_sendAutoLockSetMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    int cmd = getMsgCmd(appMsg);

    MSG_AUTOLOCK_SET_REQ *req = (MSG_AUTOLOCK_SET_REQ *)alloc_simcomAutolockSetReq(cmd, autolockSetApp2mc(cmd));
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(cmd, CODE_INTERNAL_ERR, obj->IMEI);
        return -1;
    }

    app_sendCmdRsp2App(cmd, CODE_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_AUTOLOCK_SET_REQ), obj);

    return 0;
}

static int app_sendAutoPeriodSetMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    int cmd = getMsgCmd(appMsg);

    cJSON *periodItem = cJSON_GetObjectItem(appMsg, "period");
    if (!periodItem) 
    {
        LOG_ERROR("period format error:no period item");
        return -1;
    }

    MSG_AUTOPERIOD_SET_REQ *req = (MSG_AUTOPERIOD_SET_REQ *)alloc_simcomAutoPeriodSetReq(cmd, periodItem->valueint);
    if (!req) 
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(cmd, CODE_INTERNAL_ERR, obj->IMEI);
        return -1;
    }

    app_sendCmdRsp2App(cmd, CODE_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_AUTOPERIOD_SET_REQ), obj);

    return 0;
}

static int app_sendAutoPeriodGetMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    int cmd = getMsgCmd(appMsg);

    MSG_AUTOPERIOD_GET_REQ *req = (MSG_AUTOPERIOD_GET_REQ *)alloc_simcomAutoPeriodGetReq(cmd);
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(cmd, CODE_INTERNAL_ERR, obj->IMEI);
        return -1;
    }

    app_sendCmdRsp2App(cmd, CODE_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_AUTOPERIOD_GET_REQ), obj);

    return 0;
}

static int app_sendAutoLockGetMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    int cmd = getMsgCmd(appMsg);

    MSG_AUTOLOCK_GET_REQ *req = (MSG_AUTOLOCK_GET_REQ *)alloc_simcomAutolockGetReq(cmd);
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(cmd, CODE_INTERNAL_ERR, obj->IMEI);
        return -1;
    }

    app_sendCmdRsp2App(cmd, CODE_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_AUTOLOCK_GET_REQ), obj);

    return 0;
}

static int app_sendBatteryMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    int cmd = getMsgCmd(appMsg);

    MSG_HEADER *req = (MSG_HEADER *)alloc_simcom_msg(CMD_BATTERY, sizeof(MSG_HEADER));
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(cmd, CODE_INTERNAL_ERR, obj->IMEI);
        return -1;
    }

    app_sendCmdRsp2App(cmd, CODE_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_HEADER), obj);

    return 0;
}

static int app_sendStatusGetMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    int cmd = getMsgCmd(appMsg);

    MSG_HEADER *req = (MSG_HEADER *)alloc_simcom_msg(CMD_DEVICE_INFO_GET, sizeof(MSG_HEADER));
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(cmd, CODE_INTERNAL_ERR, obj->IMEI);
        return -1;
    }

    app_sendCmdRsp2App(cmd, CODE_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_HEADER), obj);

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
    {APP_CMD_WILD,              app_sendWildMsg2Device},
    {APP_CMD_FENCE_ON,          app_sendFenceOnMsg2Device},
    {APP_CMD_FENCE_OFF,         app_sendFenceOffMsg2Device},
    {APP_CMD_FENCE_GET,         app_sendFenceGetMsg2Device},
    {APP_CMD_SEEK_ON,           app_sendSeekMsg2Device},
    {APP_CMD_SEEK_OFF,          app_sendSeekMsg2Device},
    {APP_CMD_LOCATION,          app_sendLocationMsg2Device},
    {APP_CMD_AUTOLOCK_ON,       app_sendAutoLockSetMsg2Device},
    {APP_CMD_AUTOLOCK_OFF,      app_sendAutoLockSetMsg2Device},
    {APP_CMD_AUTOPERIOD_SET,    app_sendAutoPeriodSetMsg2Device},
    {APP_CMD_AUTOPERIOD_GET,    app_sendAutoPeriodGetMsg2Device},
    {APP_CMD_AUTOLOCK_GET,      app_sendAutoLockGetMsg2Device},
    {APP_CMD_BATTERY,           app_sendBatteryMsg2Device},
    {APP_CMD_STATUS_GET,        app_sendStatusGetMsg2Device}
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

