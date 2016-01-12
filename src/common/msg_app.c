/*
 * msg_app.c
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "msg_app.h"
#include "macro.h"
#include "log.h"
#include "cJSON.h"
#include "mqtt.h"
#include "object.h"


static void app_sendMsg2App(const char *topic, const void *msg, size_t len)
{
    //LOG_HEX(data, len);
    mqtt_publish(topic, msg, len);

    return;
}

void app_sendCmdRsp2App(int cmd, int result, const char *strIMEI)
{
    char topic[IMEI_LENGTH + 13];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/cmd", strIMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", cmd);
    cJSON_AddNumberToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));

    LOG_INFO("send IMEI(%s), cmd(%d), result(%d) response to APP", strIMEI, cmd, result);
    free(json);
    cJSON_Delete(root);

    return;
}

void app_sendLocationRsp2App(int result, OBJECT* obj)
{
    char topic[IMEI_LENGTH + 13];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/cmd", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", APP_CMD_LOCATION);
    cJSON_AddNumberToObject(root, "result", result);
    cJSON_AddNumberToObject(root, "isGPSlocated", obj->isGPSlocated);
    cJSON_AddNumberToObject(root, "timestamp", obj->timestamp);
    cJSON_AddNumberToObject(root, "lat", obj->lat);
    cJSON_AddNumberToObject(root, "lng", obj->lon);
    cJSON_AddNumberToObject(root, "altitude", obj->altitude);
    cJSON_AddNumberToObject(root, "speed", obj->speed);
    cJSON_AddNumberToObject(root, "course", obj->course);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));

    LOG_INFO("send IMEI(%s), cmd(%d), result(%d) response to APP", obj->IMEI, APP_CMD_LOCATION, result);
    free(json);
    cJSON_Delete(root);

    return;
}

void app_sendFenceGetRspMsg2App(int cmd, int result, int state, void *session)
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

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send fence get response to APP" );
    free(json);
    cJSON_Delete(root);
}

void app_sendGpsMsg2App(void* session)
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
    snprintf(topic, IMEI_LENGTH + 20, "dev2app/%s/gps", obj->IMEI);

    cJSON * root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", obj->timestamp);
    cJSON_AddNumberToObject(root, "isGPSlocated", obj->isGPSlocated);
    cJSON_AddNumberToObject(root, "lat", obj->lat);
    cJSON_AddNumberToObject(root, "lng", obj->lon);
    cJSON_AddNumberToObject(root, "altitude", obj->altitude);
    cJSON_AddNumberToObject(root, "speed", obj->speed);
    cJSON_AddNumberToObject(root, "course", obj->course);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
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

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send 433 msg to APP");
    free(json);
    cJSON_Delete(root);
}

void app_sendAutolockMsg2App(int timestamp, int lock, void * session)
{
    OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
    if (!obj)
    {
        LOG_ERROR("obj null, no data to upload");
        return;
    }
    char topic[IMEI_LENGTH + 13];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/autolock", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", timestamp);
    cJSON_AddNumberToObject(root, "lock", lock);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send autolock msg to APP: %d", lock);
    free(json);
    cJSON_Delete(root);
}

void app_sendAlarmMsg2App(unsigned char type, const char *msg, void *session)
{
    OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
    if (!obj)
    {
        LOG_ERROR("obj null, no data to upload");
        return;
    }
    char topic[IMEI_LENGTH + 15];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 15, "dev2app/%s/alarm", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "type", type);
    if(msg)
    {
        cJSON_AddStringToObject(root, "msg", msg);
    }

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send alarm msg to APP");
    free(json);
    cJSON_Delete(root);
}

void app_sendDebugMsg2App(const char *msg, size_t length, void *session)
{
    OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
    if (!obj)
    {
        LOG_ERROR("object null,internal error");
        return;
    }
    char topic[IMEI_LENGTH + 15];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 15, "dev2app/%s/debug", obj->IMEI);

    app_sendMsg2App(topic, msg, length);
    LOG_INFO("send debug msg to APP");
}

