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

/* dev2app/<imei>/cmd */
void app_sendCmdRsp2App(int cmd, int code, const char *strIMEI)
{
    char topic[IMEI_LENGTH + 13];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/cmd", strIMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", cmd);
    cJSON_AddNumberToObject(root, "code", code);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));

    LOG_INFO("send cmd(%d) response to APP, imei(%s), code(%d)", cmd, strIMEI, code);
    free(json);
    cJSON_Delete(root);

    return;
}

void app_sendFenceGetRsp2App(int cmd, int code, int state, void *session)
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
    cJSON_AddNumberToObject(root, "code", code);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "state", state);
    cJSON_AddItemToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));

    LOG_INFO("send fence get response to APP, imei(%s), code(%d)", obj->IMEI, code);
    free(json);
    cJSON_Delete(root);

    return;
}

void app_sendLocationRsp2App(int code, OBJECT *obj)
{
    char topic[IMEI_LENGTH + 13];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/cmd", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", APP_CMD_LOCATION);
    cJSON_AddNumberToObject(root, "code", code);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "timestamp", obj->timestamp);
    cJSON_AddNumberToObject(result, "isGPSlocated", obj->isGPSlocated);
    cJSON_AddNumberToObject(result, "lat", obj->lat);
    cJSON_AddNumberToObject(result, "lng", obj->lon);
    cJSON_AddNumberToObject(result, "altitude", obj->altitude);
    cJSON_AddNumberToObject(result, "speed", obj->speed);
    cJSON_AddNumberToObject(result, "course", obj->course);
    cJSON_AddItemToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send location response to APP, imei(%s), code(%d)", obj->IMEI, code);
    free(json);
    cJSON_Delete(root);

    return;
}

void app_sendAutoPeriodGetRsp2App(int cmd, int code, int period, void *session)
{
    OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
    if (!obj)
    {
        LOG_ERROR("obj null, no data to upload");
        return;
    }
    char topic[IMEI_LENGTH + 13];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/cmd", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", cmd);
    cJSON_AddNumberToObject(root, "code", code);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "period", period);
    cJSON_AddItemToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send auto period get response to APP, imei(%s), code(%d)", obj->IMEI, code);
    free(json);
    cJSON_Delete(root);

    return;
}

void app_sendAutoLockGetRsp2App(int cmd, int code, int state, void *session)
{
    OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
    if (!obj)
    {
        LOG_ERROR("obj null, no data to upload");
        return;
    }
    char topic[IMEI_LENGTH + 13];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/cmd", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", cmd);
    cJSON_AddNumberToObject(root, "code", code);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "state", state);
    cJSON_AddItemToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send auto lock get response to APP, imei(%s), code(%d)", obj->IMEI, code);
    free(json);
    cJSON_Delete(root);

    return;
}

void app_sendBatteryRsp2App(int cmd, int code, int percent, int miles, void *session)
{
    OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
    if (!obj)
    {
        LOG_ERROR("obj null, no data to upload");
        return;
    }
    char topic[IMEI_LENGTH + 13];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/cmd", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", cmd);
    cJSON_AddNumberToObject(root, "code", code);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "percent", percent);
    cJSON_AddNumberToObject(result, "miles", miles);
    cJSON_AddItemToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send battery response to APP, imei(%s), code(%d)", obj->IMEI, code);
    free(json);
    cJSON_Delete(root);

    return;
}

void app_sendAutoLockNotifyRsp2App(int cmd, int code, int timestamp, int lock, void *session)
{
    OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
    if (!obj)
    {
        LOG_ERROR("obj null, no data to upload");
        return;
    }
    char topic[IMEI_LENGTH + 13];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/cmd", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", cmd);
    cJSON_AddNumberToObject(root, "code", code);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "timestamp", timestamp);
    cJSON_AddNumberToObject(result, "lock", lock);
    cJSON_AddItemToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send auto lock notify to APP, imei(%s), code(%d)", obj->IMEI, code);
    free(json);
    cJSON_Delete(root);

    return;
}

/* dev2app/<imei>/gps */
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
    snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/gps", obj->IMEI);

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
    LOG_INFO("send gps msg to APP, imei(%s)", obj->IMEI);
    free(json);
    cJSON_Delete(root);
}

/* dev2app/<imei>/433 */
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
    LOG_INFO("send 433 msg to APP, imei(%s)", obj->IMEI);
    free(json);
    cJSON_Delete(root);
}

/* dev2app/<imei>/alarm */
void app_sendAlarmMsg2App(int type, const char *msg, void *session)
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
    LOG_INFO("send alarm msg to APP, imei(%s)", obj->IMEI);
    free(json);
    cJSON_Delete(root);

    return;
}

/* dev2app/<imei>/debug */
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

    return;
}

