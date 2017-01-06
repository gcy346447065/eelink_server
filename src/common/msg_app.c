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

#define MAX_TOPIC_LEN 50

static void app_sendMsg2App(const char *topic, const void *msg, size_t len)
{
    //LOG_HEX(data, len);
    mqtt_publish(topic, msg, len);

    return;
}

/* dev2app/<imei>/cmd */
void app_sendCmdRsp2App(int cmd, int code, const char *strIMEI)
{
    char topic[MAX_TOPIC_LEN];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/cmd", strIMEI);

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

    char topic[MAX_TOPIC_LEN];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/cmd", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", cmd);
    cJSON_AddNumberToObject(root, "code", code);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "state", state);
    cJSON_AddItemToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));

    LOG_INFO("send fence get response to APP, imei(%s), code(%d), state(%d)",
             obj->IMEI, code, state);

    free(json);
    cJSON_Delete(root);

    return;
}

void app_sendLocationRsp2App(int code, OBJECT *obj)
{
    char topic[MAX_TOPIC_LEN];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/cmd", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", APP_CMD_LOCATION);
    cJSON_AddNumberToObject(root, "code", code);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "timestamp", obj->timestamp);
    cJSON_AddNumberToObject(result, "isGPSlocated", obj->isGPSlocated);
    cJSON_AddNumberToObject(result, "lat", obj->lat);
    cJSON_AddNumberToObject(result, "lng", obj->lon);
    cJSON_AddNumberToObject(result, "speed", obj->speed);
    cJSON_AddNumberToObject(result, "course", obj->course);
    cJSON_AddItemToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send location response to APP, imei(%s), code(%d), timestamp(%d), isGPSlocated(%d), lat(%f), lng(%f)",
             obj->IMEI, code, obj->timestamp, obj->isGPSlocated, obj->lat, obj->lon);

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
    char topic[MAX_TOPIC_LEN];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/cmd", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", cmd);
    cJSON_AddNumberToObject(root, "code", code);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "period", period);
    cJSON_AddItemToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send auto period get response to APP, imei(%s), code(%d), period(%d)",
             obj->IMEI, code, period);

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
    char topic[MAX_TOPIC_LEN];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/cmd", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", cmd);
    cJSON_AddNumberToObject(root, "code", code);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "state", state);
    cJSON_AddItemToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send auto lock get response to APP, imei(%s), code(%d), state(%d)",
             obj->IMEI, code, state);

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
    char topic[MAX_TOPIC_LEN];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/cmd", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", cmd);
    cJSON_AddNumberToObject(root, "code", code);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "percent", percent);
    cJSON_AddNumberToObject(result, "miles", miles);
    cJSON_AddItemToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send battery response to APP, imei(%s), code(%d), percent(%d), miles(%d)",
             obj->IMEI, code, percent, miles);

    free(json);
    cJSON_Delete(root);

    return;
}

void app_sendStatusGetRsp2App(int cmd, int code, OBJECT *obj, char autolock, char autoperiod, char percent, char miles, char status)
{
    char topic[MAX_TOPIC_LEN];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/cmd", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "cmd", cmd);
    cJSON_AddNumberToObject(root, "code", code);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddBoolToObject(result, "isGPSlocated", obj->isGPSlocated);

    cJSON *gps = cJSON_CreateObject();
    cJSON_AddNumberToObject(gps, "timestamp", obj->timestamp);
    cJSON_AddNumberToObject(gps, "lat", obj->lat);
    cJSON_AddNumberToObject(gps, "lng", obj->lon);
    cJSON_AddNumberToObject(gps, "speed", obj->speed);
    cJSON_AddNumberToObject(gps, "course", obj->course);
    cJSON_AddItemToObject(result, "gps", gps);

    cJSON_AddBoolToObject(result, "lock", status);

    cJSON *J_autolock = cJSON_CreateObject();
    cJSON_AddBoolToObject(J_autolock, "isOn", autolock);
    cJSON_AddNumberToObject(J_autolock, "period", autoperiod);
    cJSON_AddItemToObject(result, "autolock", J_autolock);

    cJSON *battery = cJSON_CreateObject();
    cJSON_AddNumberToObject(battery, "percent", percent);
    cJSON_AddNumberToObject(battery, "miles", miles);
    cJSON_AddItemToObject(result, "battery", battery);

    cJSON_AddItemToObject(root, "result", result);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send status get response to APP, imei(%s), code(%d),"
             "isGPSlocated(%d), timestamp(%d), lat(%f), lng(%f), speed(%d), course(%d),"
             "lock(%d), isOn(%d), period(%d), percent(%d), miles(%d)",
              obj->IMEI, code, obj->isGPSlocated, obj->timestamp, obj->lat, obj->lon,
              obj->speed, obj->course, status, autolock, autoperiod, percent, miles);

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

    char topic[MAX_TOPIC_LEN];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/gps", obj->IMEI);

    cJSON * root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", obj->timestamp);
    cJSON_AddNumberToObject(root, "isGPSlocated", obj->isGPSlocated);
    cJSON_AddNumberToObject(root, "lat", obj->lat);
    cJSON_AddNumberToObject(root, "lng", obj->lon);
    cJSON_AddNumberToObject(root, "speed", obj->speed);
    cJSON_AddNumberToObject(root, "course", obj->course);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send gps msg to APP, imei(%s), isGPSlocated(%d), timestamp(%d), lat(%f), lng(%f), speed(%d), course(%d),",
             obj->IMEI, obj->isGPSlocated, obj->timestamp, obj->lat, obj->lon, obj->speed, obj->course);

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
    char topic[MAX_TOPIC_LEN];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/433", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", timestamp);
    cJSON_AddNumberToObject(root, "intensity", intensity);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send 433 msg to APP, imei(%s), timestamp(%d), intensity(%d)", obj->IMEI, timestamp, intensity);
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
    char topic[MAX_TOPIC_LEN];
    memset(topic, 0, sizeof(topic));
    switch(type)
    {
        case 1:
        case 2:
        case 3:
            snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/alarm", obj->IMEI);
            break;

        case 4:
            snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/battery50%", obj->IMEI);//TODO:protocol
            break;

        case 5:
            snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/battery30%", obj->IMEI);//TODO:protocol
            break;

        default:
            break;

    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "type", type);
    if(msg)
    {
        cJSON_AddStringToObject(root, "msg", msg);
    }

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send alarm msg to APP, imei(%s), type(%d)", obj->IMEI, type);

    free(json);
    cJSON_Delete(root);

    return;
}

/* dev2app/<imei>/ftp */
void app_sendFTPPutEndMsg2App(char code, char *fileName, void *session)
{
    OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
    if (!obj)
    {
        LOG_ERROR("obj null, no data to upload");
        return;
    }
    char topic[MAX_TOPIC_LEN];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/ftp", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "code", code);
    cJSON_AddStringToObject(root, "fileName", fileName);
    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send ftp put end msg to APP, imei(%s), code(%d) file(%s)", obj->IMEI, code, fileName);

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
    char topic[MAX_TOPIC_LEN];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/debug", obj->IMEI);

    app_sendMsg2App(topic, msg, length);
    LOG_INFO("send debug msg to APP, imei(%s)", obj->IMEI);

    return;
}

/* dev2app/<imei>/notify */
void app_sendNotifyMsg2App(int notify, int timestamp, int lock_status, void *session)
{
    OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
    if (!obj)
    {
        LOG_ERROR("object null,internal error");
        return;
    }
    char topic[MAX_TOPIC_LEN];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, MAX_TOPIC_LEN, "dev2app/%s/notify", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "notify", notify);

    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "timestamp", timestamp);
    if(notify == NOTIFY_AUTOLOCK)
    {
        cJSON_AddNumberToObject(data, "lock", lock_status);
    }
    else if(notify == NOTIFY_STATUS)
    {
        cJSON_AddNumberToObject(data, "status", lock_status);
    }
    cJSON_AddItemToObject(root, "data", data);

    char *json = cJSON_PrintUnformatted(root);

    app_sendMsg2App(topic, json, strlen(json));
    LOG_INFO("send notify msg to APP, imei(%s), notify(%d), timestamp(%d), lock_status(%d)",
             obj->IMEI, notify, timestamp, lock_status);

    free(json);
    cJSON_Delete(root);

    return;
}
