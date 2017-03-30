/*
 * http_simcom.c
 *
 *  Created on: 2016/12/03
 *      Author: lc
 */
#include <malloc.h>
#include <string.h>

#include "http_simcom.h"
#include "session.h"
#include "log.h"
#include "cJSON.h"
#include "object.h"
#include "protocol.h"
#include "timer.h"
#include "msg_simcom.h"
#include "msg_http.h"
#include "request_table.h"
#include "msg_proc_simcom.h"

typedef struct
{
    GHashTable *request_table;
    unsigned char seq;
}REQ_EVENT;

static void device_timeout_cb(evutil_socket_t fd __attribute__((unused)), short what __attribute__((unused)), void *arg)
{
    REQ_EVENT *req_event = arg;

    if(!req_event->request_table || !req_event->seq)
    {
        free(req_event);
        return;
    }

    struct evhttp_request *req = request_get(req_event->request_table, req_event->seq);
    if(req)
    {
        http_errorReply(req, CODE_DEVICE_NO_RSP);
        request_del(req_event->request_table, req_event->seq);
    }
    free(req_event);
    return;
}

static void simcom_deviceHandler(struct evhttp_request *req)
{
    char post_data[MAX_MSGHTTP_LEN] = {0};
    evbuffer_copyout(req->input_buffer,post_data,MAX_MSGHTTP_LEN);

    LOG_INFO("get the request from http:%s", post_data);

    cJSON *json = cJSON_Parse(post_data);
    if(!json)
    {
        LOG_ERROR("get data is not json type");
        http_errorReply(req, CODE_ERROR_CONTENT);
        return;
    }

    cJSON *imei = cJSON_GetObjectItem(json, "imei");
    if(!imei)
    {
        LOG_ERROR("no imei in data");
        cJSON_Delete(json);
        http_errorReply(req, CODE_ERROR_CONTENT);
        return;
    }

    OBJECT *obj = obj_get(imei->valuestring);
    if(!obj)
    {
        LOG_WARN("object not exists");
        cJSON_Delete(json);
        http_errorReply(req, CODE_IMEI_NOT_FOUND);
        return;
    }

    SESSION *session = obj->session;
    if(!session)
    {
        LOG_ERROR("device offline");
        cJSON_Delete(json);
        http_errorReply(req, CODE_DEVICE_OFF);
        return;
    }

    MSG_SEND pfn = session->pSendMsg;
    if (!pfn)
    {
        LOG_ERROR("device offline");
        http_errorReply(req, CODE_DEVICE_OFF);
        return;
    }

    cJSON *cmd = cJSON_GetObjectItem(json, "cmd");
    if(!cmd)
    {
        LOG_ERROR("no cmd in data");
        cJSON_Delete(json);
        http_errorReply(req, CODE_ERROR_CONTENT);
        return;
    }

    char *data = cJSON_PrintUnformatted(cmd);
    if(!data)
    {
        LOG_ERROR("no memory");
        cJSON_Delete(json);
        http_errorReply(req, CODE_INTERNAL_ERROR);
        return;
    }
    cJSON_Delete(json);

    int msgLen = sizeof(MSG_DEVICE_REQ) + strlen(data);

    MSG_DEVICE_REQ *msg = alloc_device_msg(CMD_DEVICE, obj->request_seq, msgLen);
    if(!msg)
    {
        LOG_ERROR("no memory");
        cJSON_Delete(json);
        http_errorReply(req, CODE_INTERNAL_ERROR);
        free(data);
        return;
    }
    strncpy(msg->data, data, strlen(data));
    free(data);

    LOG_HEX(msg, sizeof(MSG_DEVICE_REQ));
    pfn(session->bev, msg, msgLen); //simcom_sendMsg

    free_simcom_msg(msg);

    request_add(obj->request_table, req, obj->request_seq);

    REQ_EVENT *req_event = (REQ_EVENT *)malloc(sizeof(REQ_EVENT));

    req_event->request_table = obj->request_table;
    req_event->seq = obj->request_seq++;

    //set a timer to response to http if request can't get response from device.
    struct timeval tv = {4, 5000};// X.005 seconds
    timer_newOnce(session->base, &tv, device_timeout_cb, req_event);

    return;
}


static void simcom_deviceData(struct evhttp_request *req)
{
    char imei[MAX_IMEI_LENGTH + 1] = {0};
    int rc = 0;
    LOG_INFO("%s", req->uri);

    rc = sscanf(req->uri, "/v1/imeiData/%15s%*s", imei);
    if(rc != 1)
    {
        http_errorReply(req, CODE_IMEI_NOT_FOUND);
        return;
    }
    LOG_INFO("%s", imei);
    OBJECT *obj = obj_get(imei);
    if(!obj)
    {
        LOG_WARN("object not exists");
        http_errorReply(req, CODE_IMEI_NOT_FOUND);
        return;
    }

    cJSON *root = cJSON_CreateObject();
    if(!root)
    {
        http_errorReply(req, CODE_INTERNAL_ERROR);
        return;
    }

    cJSON_AddStringToObject(root, "imei", obj->IMEI);
    cJSON_AddNumberToObject(root, "version", obj->version);
    cJSON_AddNumberToObject(root, "timestamp", obj->timestamp);
    cJSON_AddNumberToObject(root, "latitude", obj->lat);
    cJSON_AddNumberToObject(root, "longitude", obj->lon);
    cJSON_AddNumberToObject(root, "course", obj->course);
    cJSON_AddNumberToObject(root, "speed", obj->speed);
    cJSON_AddNumberToObject(root, "GSM", obj->gsm);
    cJSON_AddNumberToObject(root, "MAXGSM", obj->max_gsm);
    cJSON_AddNumberToObject(root, "voltage", obj->voltage);

    SESSION *session = obj->session;
    if(session && session->pSendMsg)
    {
        cJSON_AddNumberToObject(root, "state", 1);
    }
    else
    {
        cJSON_AddNumberToObject(root, "state", 0);
    }


    char *data = cJSON_PrintUnformatted(root);
    if(!data)
    {
        LOG_ERROR("internal error");
        http_errorReply(req, CODE_INTERNAL_ERROR);
    }
    else
    {
        LOG_INFO("%s", data);
        http_postReply(req, data);
        free(data);
    }
    cJSON_Delete(root);

    return;
}

enum
{
    SERVER_UPGRADEDEVICE = 0
}SERVER_CMD_HANDLER;

static void simcom_serverHandler(struct evhttp_request *req)
{
    int c = 0;
    char post_data[MAX_MSGHTTP_LEN] = {0};
    evbuffer_copyout(req->input_buffer,post_data,MAX_MSGHTTP_LEN);

    LOG_INFO("get the request from http:%s", post_data);

    cJSON *json = cJSON_Parse(post_data);
    if(!json)
    {
        LOG_ERROR("get data is not json type");
        http_errorReply(req, CODE_ERROR_CONTENT);
        return;
    }

    cJSON *imei = cJSON_GetObjectItem(json, "imei");
    if(!imei)
    {
        LOG_ERROR("no imei in data");
        cJSON_Delete(json);
        http_errorReply(req, CODE_ERROR_CONTENT);
        return;
    }

    OBJECT *obj = obj_get(imei->valuestring);
    if(!obj)
    {
        LOG_WARN("object not exists");
        cJSON_Delete(json);
        http_errorReply(req, CODE_IMEI_NOT_FOUND);
        return;
    }

    cJSON *cmd = cJSON_GetObjectItem(json, "cmd");
    if(!cmd)
    {
        LOG_ERROR("no cmd in data");
        cJSON_Delete(json);
        http_errorReply(req, CODE_ERROR_CONTENT);
        return;
    }
    c = cmd->valueint;
    cJSON_Delete(json);

    switch(c)
    {
        case SERVER_UPGRADEDEVICE:
            {
                SESSION *session = obj->session;
                if(!session)
                {
                    LOG_ERROR("device offline");
                    cJSON_Delete(json);
                    http_errorReply(req, CODE_DEVICE_OFF);
                    return;
                }

                MSG_SEND pfn = session->pSendMsg;
                if (!pfn)
                {
                    LOG_ERROR("device offline");
                    http_errorReply(req, CODE_DEVICE_OFF);
                    return;
                }
                simcom_startUpgradeRequestObligatory(obj);
            }
            break;
        default:
            http_errorReply(req, CODE_RANGE_TOO_LARGE);
            return;
    }

    http_postReply(req, "{\"code\":0}");
    return;

}


void simcom_http_handler(struct evhttp_request *req, void *arg __attribute__((unused)))
{
    LOG_INFO("%s", req->uri);
    switch(req->type)
    {
        case EVHTTP_REQ_POST:
            if(strstr(req->uri, "/v1/device"))
            {
                simcom_deviceHandler(req);
                return;
            }
            if(strstr(req->uri, "/v1/server"))
            {
                simcom_serverHandler(req);
                return;
            }
            break;

        case EVHTTP_REQ_GET:
            if(strstr(req->uri, "/v1/imeiData"))
            {
                simcom_deviceData(req);
                return;
            }
            break;

        case EVHTTP_REQ_PUT:
        case EVHTTP_REQ_DELETE:
        default:
            break;
    }
    http_errorReply(req, CODE_URL_ERR);
    return;
}


