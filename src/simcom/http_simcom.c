/*
 * http_simcom.c
 *
 *  Created on: 2016/12/03
 *      Author: lc
 */
#include <string.h>

#include "http_simcom.h"
#include "session.h"
#include "log.h"
#include "cJSON.h"
#include "object.h"
#include "protocol.h"

#define MSG_MAX_LEN 1024

static void simcom_deviceHandler(struct evhttp_request *req)
{
    char post_data[MSG_MAX_LEN] = {0};
    evbuffer_copyout(req->input_buffer,post_data,MSG_MAX_LEN);

    LOG_INFO("get the request from http:%s", post_data);

    cJSON *json = cJSON_Parse(post_data);
    if(!json)
    {
        LOG_ERROR("get data is not json type");
        simcom_errorHttp(req, CODE_ERROR_CONTENT);
        return;
    }

    cJSON *imei = cJSON_GetObjectItem(json, "imei");
    if(!imei)
    {
        LOG_ERROR("no imei in data");
        cJSON_Delete(json);
        simcom_errorHttp(req, CODE_ERROR_CONTENT);
        return;
    }

    OBJECT *obj = obj_get(imei->valuestring);
    if(!obj)
    {
        LOG_WARN("object not exists");
        cJSON_Delete(json);
        simcom_errorHttp(req, CODE_IMEI_NOT_FOUND);
        return;
    }

    SESSION *session = obj->session;
    if(!session)
    {
        LOG_ERROR("device offline");
        cJSON_Delete(json);
        simcom_errorHttp(req, CODE_DEVICE_OFFLINE);
        return;
    }

    MSG_SEND pfn = session->pSendMsg;
    if (!pfn)
    {
        LOG_ERROR("device offline");
        simcom_errorHttp(req, CODE_DEVICE_OFFLINE);
        return;
    }

    cJSON *param = cJSON_GetObjectItem(json, "param");
    if(!param)
    {
        LOG_ERROR("no param in data");
        cJSON_Delete(json);
        simcom_errorHttp(req, CODE_ERROR_CONTENT);
        return;
    }

    cJSON *action = cJSON_GetObjectItem(json, "action");
    if(!action)
    {
        LOG_ERROR("no param in data");
        cJSON_Delete(json);
        simcom_errorHttp(req, CODE_ERROR_CONTENT);
        return;
    }

    char *data = cJSON_PrintUnformatted(param);
    if(!data)
    {
        LOG_ERROR("no memory");
        cJSON_Delete(json);
        simcom_errorHttp(req, CODE_INTERNAL_ERR);
        return;
    }

    int msgLen = sizeof(MSG_DEVICE_REQ) + strlen(data);

    MSG_DEVICE_REQ *msg = alloc_simcom_msg(CMD_DEVICE, msgLen);
    if(!msg)
    {
        LOG_ERROR("no memory");
        cJSON_Delete(json);
        simcom_errorHttp(req, CODE_INTERNAL_ERR);
        free(data);
        return;
    }

    msg->action = action->valueint;
    strncpy(msg->data, data, strlen(data));

    LOG_HEX(msg, sizeof(MSG_DEVICE_REQ));
    pfn(session->bev, msg, msgLen); //simcom_sendMsg

    //TODO:set a timer to response to http if request can't get response.

    session->req = req;
    cJSON_Delete(json);
    free(data);
    return;
}

void simcom_http_handler(struct evhttp_request *req, void *arg __attribute__((unused)))
{
    if(strstr(req->uri, "/v1/device"))
    {
        simcom_deviceHandler(req);
    }
    return;
}

void simcom_replyHttp(struct evhttp_request *req, const char *data)
{
    evhttp_add_header(req->output_headers, "Server", "simcom v1");
    evhttp_add_header(req->output_headers, "Content-Type", "application/json");
    evhttp_add_header(req->output_headers, "Connection", "close");
    struct evbuffer *buf = evbuffer_new();
    if(data)
    {
        evbuffer_add_printf(buf, "%s", data);
    }
    else
    {
        evbuffer_add_printf(buf, "%s", "{\"code\":101}");
    }
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
    return;
}

void simcom_errorHttp(struct evhttp_request *req, char errorType)
{
    char buf[32] = {0};
    snprintf(buf, 32, "{\"code\":%d},errorType");
}

