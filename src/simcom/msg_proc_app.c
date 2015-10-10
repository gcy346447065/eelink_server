/*
 * msg_proc_app.c
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <netinet/in.h>

#include "msg_app.h"
#include "msg_simcom.h"
#include "msg_proc_simcom.h"
#include "log.h"
#include "macro.h"
#include "session.h"
#include "object.h"
#include "mqtt.h"
#include "protocol.h"
#include "cJSON.h"


 //----------------------------send raw msg to app/mc------------------------------------------
static void app_sendRawData2mc(const void* msg, size_t len, const char* imei)
{
    SESSION *session = session_get(imei);
    if(!session)
    {
        LOG_ERROR("obj %s offline", imei);
        return;
    }
    int rc = simcom_msg_send(msg, len, session);
    if(rc)
    {
        LOG_ERROR("send msg to simcom error");
    }
}

static void app_sendRawData2App(const char* topic, const void *msg, size_t len)
{
//  LOG_HEX(data, len);
    mqtt_publish(topic, msg, len);
}


//----------------------------send cmd/gps/433 to app-----------------------------------------
void app_sendCmdMsg2App(int cmd, int result, const char *strIMEI)
{
    /*
    if (!session)
    {
        LOG_FATAL("internal error: session null");
        return;
    }

    OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
    if (!obj)
    {
        LOG_FATAL("internal error: obj null");
        return;
    }
*/
    char topic[IMEI_LENGTH + 13];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/cmd", strIMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", cmd);
    cJSON_AddNumberToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendRawData2App(topic, json, strlen(json));
    LOG_INFO("send cmd msg to APP" );
    free(json);
    cJSON_Delete(root);
}

void app_sendFenceGetCmdMsg2App(int cmd, int result, int state, void* session)
{
    if (!session)
    {
        LOG_FATAL("internal error: session null");
        return;
    }

    OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
    if (!obj)
    {
        LOG_FATAL("internal error: obj null");
        return;
    }

    char topic[IMEI_LENGTH + 13];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/cmd", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", cmd);
    cJSON_AddNumberToObject(root, "result", result);
    cJSON_AddNumberToObject(root, "state", state);

    char *json = cJSON_PrintUnformatted(root);

    app_sendRawData2App(topic, json, strlen(json));
    LOG_INFO("send cmd msg to APP" );
    free(json);
    cJSON_Delete(root);
}

void app_sendGpsMsg2App(void* session)
{
    OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
    if (!obj)
    {
        LOG_ERROR("obj null, no data to upload");
        return;
    }
    char topic[IMEI_LENGTH + 13];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 20, "dev2app/%s/gps", obj->IMEI);

    cJSON * root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", obj->timestamp);
    cJSON_AddNumberToObject(root, "lat", obj->lat);
    cJSON_AddNumberToObject(root, "lng", obj->lon);

    char *json = cJSON_PrintUnformatted(root);

    app_sendRawData2App(topic, json, strlen(json));
    LOG_INFO("send gps msg to APP");
    free(json);
    cJSON_Delete(root);
}

void app_send433Msg2App(int timestamp, int intensity, void * session)
{
    OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
    if (!obj)
    {
        LOG_ERROR("obj null, no data to upload");
        return;
    }
    char topic[IMEI_LENGTH + 13];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/433", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", timestamp);
    cJSON_AddNumberToObject(root, "intensity", intensity);

    char *json = cJSON_PrintUnformatted(root);

    app_sendRawData2App(topic, json, strlen(json));
    LOG_INFO("send 433 msg to APP");
    free(json);
    cJSON_Delete(root);
}


//---------------------------------handle for msg from app------------------------------------
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

int app_handleApp2devMsg(const char* topic, const char* data, const int len, void* userdata)
{
    if (!data)
    {
        LOG_FATAL("internal error: data null");
        return -1;
    }

    LOG_DEBUG("topic = %s, payload = %s", topic, data);

    //check the IMEI
    const char* pStart = &topic[strlen("app2dev/")];
    const char* pEnd = strstr(pStart, "/");
    char strIMEI[IMEI_LENGTH + 1] = {0};
    if (pEnd - pStart != IMEI_LENGTH)
    {
        LOG_ERROR("app2dev: imei length has a problem");
        return -1;
    }

    strncpy(strIMEI, pStart, IMEI_LENGTH);

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

    SESSION *ctx = session_get(strIMEI);
    if(!ctx)
    {
        LOG_ERROR("simcom %s offline", strIMEI);
        app_sendCmdMsg2App(cmd, ERR_OFFLINE, strIMEI);
        return -1;
    }

    switch (cmd)
    {
    case APP_CMD_WILD:
        LOG_INFO("receive app wildcard cmd");
        //app_sendRawData2mc(pMsg->data, ntohs(pMsg->length) - sizeof(pMsg->seq), ctx, token);
        break;
    case APP_CMD_FENCE_ON:
    case APP_CMD_FENCE_OFF:
    case APP_CMD_FENCE_GET:
    {
        LOG_INFO("receive app APP_CMD_FENCE_%d", cmd);

        MSG_DEFEND_REQ *req = (MSG_DEFEND_REQ *)alloc_simcom_msg(CMD_DEFEND, sizeof(MSG_DEFEND_REQ));
        if(!req)
        {
            LOG_FATAL("insufficient memory");
            return -1;
        }
        req->token = cmd;
        req->operator = defendApp2mc(cmd);
        app_sendCmdMsg2App(cmd, ERR_WAITING, strIMEI);
        app_sendRawData2mc(req, sizeof(MSG_DEFEND_REQ), strIMEI);
        break;
    }
    case APP_CMD_SEEK_ON:
    case APP_CMD_SEEK_OFF:
    {
        LOG_INFO("receive app APP_CMD_SEEK_%d", cmd);

        MSG_SEEK_REQ *req = (MSG_SEEK_REQ *)alloc_simcom_msg(CMD_SEEK, sizeof(MSG_SEEK_REQ));
        if(!req)
        {
            LOG_FATAL("insufficient memory");
            return -1;
        }
        req->token = cmd;
        req->operator = seekApp2mc(cmd);
        app_sendCmdMsg2App(cmd, ERR_WAITING, strIMEI);
        app_sendRawData2mc(req, sizeof(MSG_SEEK_REQ), strIMEI);
        break;
    }
    case APP_CMD_LOCATION:
    {
        LOG_INFO("receive app APP_CMD_LOCATION");
        MSG_LOCATION *req = (MSG_LOCATION *)alloc_simcom_msg(CMD_LOCATION, sizeof(MSG_LOCATION));
        if(!req)
        {
            LOG_FATAL("insufficient memory");
            return -1;
        }
        app_sendCmdMsg2App(cmd, ERR_WAITING, strIMEI);
        app_sendRawData2mc(req, sizeof(MSG_LOCATION), strIMEI);
        break;
    }
    default:
        LOG_ERROR("Unknown cmd: %#x", cmdItem->valuestring);
        break;
    }
    return 0;
}

