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
    if (!pfn)
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
    else
    {
        return DEFEND_GET;
    }
}

static char seekApp2mc(int cmd)
{
    if(cmd == APP_CMD_SEEK_ON)
    {
        return SEEK_ON;
    }
    else
    {
        return SEEK_OFF;
    }
}

static int getMsgCmd(cJSON* appMsg)
{
    cJSON* cmdItem = cJSON_GetObjectItem(appMsg, "cmd");
    if (!cmdItem)
    {
        LOG_ERROR("command format error:no cmd item");
        return -1;
    }

    return cmdItem->valueint;
}

static int app_sendWildMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    cJSON *dataItem = cJSON_GetObjectItem(appMsg, "data");
    if (!dataItem) 
    {
        LOG_ERROR("wild cmd with no data");
        return -1;
    }
    char* data = dataItem->string;

    void *msg = alloc_simcomWildMsg(data, strlen(data));
    if (!msg)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(APP_CMD_WILD, ERR_INTERNAL, obj->IMEI);
        return -1;
    }

    app_sendMsg2Device(msg, MSG_HEADER_LEN + strlen(data), obj);
    return 0;
}

static int app_sendFenceMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    int cmd = getMsgCmd(appMsg);

    MSG_DEFEND_REQ *req = (MSG_DEFEND_REQ *)alloc_simcomDefendReq(cmd, defendApp2mc(cmd));
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(cmd, ERR_INTERNAL, obj->IMEI);
        return -1;
    }

    app_sendCmdRsp2App(cmd, ERR_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_DEFEND_REQ), obj);
    return 0;
}

static int app_sendSeekMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    int cmd = getMsgCmd(appMsg);

    //TODO: the following should be encapsulate like alloc_simcomDefendReq
    MSG_SEEK_REQ *req = (MSG_SEEK_REQ *)alloc_simcom_msg(CMD_SEEK, sizeof(MSG_SEEK_REQ));
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(cmd, ERR_INTERNAL, obj->IMEI);
        return -1;
    }
    req->token = cmd;
    req->operator = seekApp2mc(cmd);
    app_sendCmdRsp2App(cmd, ERR_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_SEEK_REQ), obj);
    return 0;
}

static int app_sendLocationMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    appMsg = appMsg;

    //TODO: the following should be encapsulate like alloc_simcomDefendReq
    MSG_LOCATION *req = (MSG_LOCATION *)alloc_simcom_msg(CMD_LOCATE, sizeof(MSG_LOCATION));
    if (!req)
    {
        LOG_FATAL("insufficient memory");

        app_sendLocationRsp2App(ERR_INTERNAL, obj);
        return -1;
    }

    app_sendLocationRsp2App(ERR_WAITING, obj);
    app_sendMsg2Device(req, sizeof(MSG_LOCATION), obj);
    return 0;
}

static int app_sendAutoLockOnMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    int cmd = getMsgCmd(appMsg);

    //TODO: the following should be encapsulate like alloc_simcomDefendReq
    MSG_AUTODEFEND_SWITCH_SET_REQ *req = (MSG_AUTODEFEND_SWITCH_SET_REQ *)alloc_simcom_msg(CMD_SET_AUTOSWITCH,
                                                                                           sizeof(MSG_AUTODEFEND_SWITCH_SET_REQ));
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(cmd, ERR_INTERNAL, obj->IMEI);
        return -1;
    }

    req->token = APP_CMD_AUTOLOCK_ON;
    req->onOff = AUTO_DEFEND_ON;

    app_sendCmdRsp2App(cmd, ERR_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_AUTODEFEND_SWITCH_SET_REQ), obj);

    return 0;
}

static int app_sendAutoLockOffMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    int cmd = getMsgCmd(appMsg);

    //TODO: the following should be encapsulate like alloc_simcomDefendReq
    MSG_AUTODEFEND_SWITCH_SET_REQ *req = (MSG_AUTODEFEND_SWITCH_SET_REQ *)alloc_simcom_msg(CMD_SET_AUTOSWITCH,
                                                                                           sizeof(MSG_AUTODEFEND_SWITCH_SET_REQ));
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(cmd, ERR_INTERNAL, obj->IMEI);
        return -1;
    }
    req->token = APP_CMD_AUTOLOCK_OFF;
    req->onOff = AUTO_DEFEND_OFF;

    app_sendCmdRsp2App(cmd, ERR_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_AUTODEFEND_SWITCH_SET_REQ), obj);

    return 0;
}

static int app_sendAutoPeriodSetMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    int cmd = getMsgCmd(appMsg);


    //TODO: the following should be encapsulate like alloc_simcomDefendReq
    MSG_AUTODEFEND_PERIOD_SET_REQ *req = (MSG_AUTODEFEND_PERIOD_SET_REQ *)alloc_simcom_msg(CMD_SET_PERIOD,
                                                                                           sizeof(MSG_AUTODEFEND_PERIOD_SET_REQ));
    if (!req) {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(cmd, ERR_INTERNAL, obj->IMEI);
        return -1;
    }

    cJSON *periodItem = cJSON_GetObjectItem(appMsg, "period");
    if (!periodItem) {
        LOG_ERROR("period format error:no period item");
        return -1;
    }

    int period = periodItem->valueint;

    req->token = APP_CMD_AUTOPERIOD_SET;
    req->period = period;

    app_sendCmdRsp2App(cmd, ERR_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_AUTODEFEND_PERIOD_SET_REQ), obj);

    return 0;

}

static int app_sendAutoPeriodGetMsg2Device(cJSON* appMsg, OBJECT* obj)
{
    int cmd = getMsgCmd(appMsg);


    //TODO: the following should be encapsulate like alloc_simcomDefendReq
    MSG_AUTODEFEND_PERIOD_GET_REQ *req = alloc_simcom_msg(CMD_SET_PERIOD,
                                                          sizeof(MSG_AUTODEFEND_PERIOD_GET_REQ));
    if (!req)
    {
        LOG_FATAL("insufficient memory");
        app_sendCmdRsp2App(cmd, ERR_INTERNAL, obj->IMEI);
        return -1;
    }

    req->token = APP_CMD_AUTOPERIOD_SET;

    app_sendCmdRsp2App(cmd, ERR_WAITING, obj->IMEI);
    app_sendMsg2Device(req, sizeof(MSG_AUTODEFEND_PERIOD_GET_REQ), obj);

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

APP_MSG_PROC_MAP msg_proc_map[] = {
        {APP_CMD_WILD,           app_sendWildMsg2Device},
        {APP_CMD_FENCE_ON,       app_sendFenceMsg2Device},
        {APP_CMD_FENCE_OFF,      app_sendFenceMsg2Device},
        {APP_CMD_FENCE_GET,      app_sendFenceMsg2Device},
        {APP_CMD_SEEK_ON,        app_sendSeekMsg2Device},
        {APP_CMD_SEEK_OFF,       app_sendSeekMsg2Device},
        {APP_CMD_LOCATION,       app_sendLocationMsg2Device},
        {APP_CMD_AUTOLOCK_ON,    app_sendAutoLockOnMsg2Device},
        {APP_CMD_AUTOLOCK_OFF,   app_sendAutoLockOffMsg2Device},
        {APP_CMD_AUTOPERIOD_SET, app_sendAutoPeriodSetMsg2Device},
        {APP_CMD_AUTOPERIOD_GET, app_sendAutoPeriodGetMsg2Device},
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
    static char strIMEI[IMEI_LENGTH + 1] = {0};
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

    /* if offline, send ERR_OFFLINE;
       if offline with APP_CMD_LOCATION, send ERR_OFFLINE with GPS;
     */
    if(!(obj->session))
    {
        LOG_WARN("simcom %s offline", strIMEI);

        if(cmd != APP_CMD_LOCATION)
        {
            app_sendCmdRsp2App(cmd, ERR_OFFLINE, strIMEI);
        }
        else
        {
            app_sendLocationRsp2App(ERR_OFFLINE, obj);
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

