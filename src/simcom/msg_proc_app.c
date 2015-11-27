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

static void app_sendMsg2Device(const void *msg, size_t len, OBJECT *obj)
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

    free(msg);

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

static char* getImeiFromTopic(const char* topic, char* IMEI)
{

    const char* pStart = &topic[strlen("app2dev/")];
    const char* pEnd = strstr(pStart, "/");

    if (pEnd - pStart > IMEI_LENGTH)
    {
        LOG_ERROR("app2dev: imei length too long");
        return -1;
    }

    strncpy(IMEI, pStart, pEnd - pStart);
}

int app_handleApp2devMsg(const char* topic, const char* data, const int len, void* userdata)
{
    if (!data)
    {
        LOG_FATAL("internal error: payload null");
        return -1;
    }

    LOG_DEBUG("topic = %s, payload = %s", topic, data);

    static char strIMEI[IMEI_LENGTH + 1] = {0};

    getImeiFromTopic(topic, strIMEI);

    OBJECT* obj = obj_get(strIMEI);
    if (!obj)
    {
        LOG_ERROR("obj %s not exist", strIMEI);
        return -1;
    }

    cJSON* appMsg = cJSON_Parse(data);
    if (!appMsg)
    {
        LOG_ERROR("app message format not json: %s", cJSON_GetErrorPtr());
        return -1;
    }
    cJSON* cmdItem = cJSON_GetObjectItem(appMsg, "cmd");
    if (!cmdItem)
    {
        LOG_ERROR("command format error:no cmd item");
        return -1;
    }

    int cmd = cmdItem->valueint;

    if(!(obj->session))
    {
        LOG_WARN("simcom %s offline", strIMEI);
        app_sendCmdRsp2App(cmd, ERR_OFFLINE, strIMEI);
        return 0;
    }

    switch (cmd)
    {
    case APP_CMD_WILD:
        LOG_INFO("receive app wildcard cmd");
        //app_sendMsg2Device(pMsg->data, ntohs(pMsg->length) - sizeof(pMsg->seq), ctx, token);
        break;
    case APP_CMD_FENCE_ON:
    case APP_CMD_FENCE_OFF:
    case APP_CMD_FENCE_GET:
    {
        LOG_INFO("receive app APP_CMD_FENCE_%d", cmd);

        MSG_DEFEND_REQ *req = alloc_simcomDefendReq(cmd, defendApp2mc(cmd));
        if(!req)
        {
            LOG_FATAL("insufficient memory");
            app_sendCmdRsp2App(cmd, ERR_INTERNAL, strIMEI);
            return -1;
        }

        app_sendCmdRsp2App(cmd, ERR_WAITING, strIMEI);
        app_sendMsg2Device(req, sizeof(MSG_DEFEND_REQ), obj);
        break;
    }
    case APP_CMD_SEEK_ON:
    case APP_CMD_SEEK_OFF:
    {
        LOG_INFO("receive app APP_CMD_SEEK: %d", cmd);

        MSG_SEEK_REQ *req = (MSG_SEEK_REQ *)alloc_simcom_msg(CMD_SEEK, sizeof(MSG_SEEK_REQ));
        if(!req)
        {
            LOG_FATAL("insufficient memory");
            app_sendCmdRsp2App(cmd, ERR_INTERNAL, strIMEI);
            return -1;
        }
        req->token = cmd;
        req->operator = seekApp2mc(cmd);
        app_sendCmdRsp2App(cmd, ERR_WAITING, strIMEI);
        app_sendMsg2Device(req, sizeof(MSG_SEEK_REQ), obj);
        break;
    }
    case APP_CMD_LOCATION:
    {
        LOG_INFO("receive app APP_CMD_LOCATION");
        MSG_LOCATION *req = alloc_simcom_msg(CMD_LOCATION, sizeof(MSG_LOCATION));
        if(!req)
        {
            LOG_FATAL("insufficient memory");
            app_sendCmdRsp2App(cmd, ERR_INTERNAL, strIMEI);
            return -1;
        }
        app_sendCmdRsp2App(cmd, ERR_WAITING, strIMEI);
        app_sendMsg2Device(req, sizeof(MSG_LOCATION), obj);
        break;
    }

        case APP_CMD_AUTOLOCK_ON:
        {
            LOG_INFO("receive app APP_CMD_AUTOLOCK_ON");
            MSG_AUTODEFEND_SWITCH_SET_REQ *req = alloc_simcom_msg(CMD_AUTODEFEND_SWITCH_SET, sizeof(MSG_AUTODEFEND_SWITCH_SET_REQ));
            if(!req)
            {
                LOG_FATAL("insufficient memory");
                app_sendCmdRsp2App(cmd, ERR_INTERNAL, strIMEI);
                return -1;
            }

            req->token = APP_CMD_AUTOLOCK_ON;
            req->onOff = AUTO_DEFEND_ON;

            app_sendCmdRsp2App(cmd, ERR_WAITING, strIMEI);
            app_sendMsg2Device(req, sizeof(MSG_AUTODEFEND_SWITCH_SET_REQ), obj);
            break;
        }

        case APP_CMD_AUTOLOCK_OFF:
        {
            LOG_INFO("receive app APP_CMD_AUTOLOCK_OFF");
            MSG_AUTODEFEND_SWITCH_SET_REQ *req = alloc_simcom_msg(CMD_AUTODEFEND_SWITCH_SET, sizeof(MSG_AUTODEFEND_SWITCH_SET_REQ));
            if(!req)
            {
                LOG_FATAL("insufficient memory");
                app_sendCmdRsp2App(cmd, ERR_INTERNAL, strIMEI);
                return -1;
            }
            req->token = APP_CMD_AUTOLOCK_OFF;
            req->onOff = AUTO_DEFEND_OFF;

            app_sendCmdRsp2App(cmd, ERR_WAITING, strIMEI);
            app_sendMsg2Device(req, sizeof(MSG_AUTODEFEND_SWITCH_SET_REQ), obj);
            break;
        }

        case APP_CMD_AUTOPERIOD_SET:
        {
            LOG_INFO("receive app APP_CMD_AUTOPERIOD_SET");
            MSG_AUTODEFEND_PERIOD_SET_REQ *req = alloc_simcom_msg(CMD_AUTODEFEND_PERIOD_SET, sizeof(MSG_AUTODEFEND_PERIOD_SET_REQ));
            if(!req)
            {
                LOG_FATAL("insufficient memory");
                app_sendCmdRsp2App(cmd, ERR_INTERNAL, strIMEI);
                return -1;
            }

            cmdItem = cJSON_GetObjectItem(appMsg, "period");
            if (!cmdItem)
            {
                LOG_ERROR("period format error:no period item");
                return -1;
            }

            int period = cmdItem->valueint;

            req->token = APP_CMD_AUTOPERIOD_SET;
            req->period = period;

            app_sendCmdRsp2App(cmd, ERR_WAITING, strIMEI);
            app_sendMsg2Device(req, sizeof(MSG_AUTODEFEND_PERIOD_SET_REQ), obj);
            break;
        }

        default:
        LOG_ERROR("Unknown cmd: %#x", cmdItem->valuestring);
        break;
    }
    return 0;
}

