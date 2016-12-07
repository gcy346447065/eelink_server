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
#include "timer.h"
#include "msg_simcom.h"
#include "msg_http.h"

typedef struct
{
    SESSION *session;
    unsigned char seq;
}REQ_EVENT;

static void timer_cb(evutil_socket_t fd __attribute__((unused)), short what __attribute__((unused)), void *arg)
{
    REQ_EVENT *reqsession = arg;
    REQLIST *reqlist = find_reqList(reqsession->session->reqList, reqsession->seq);
    if(reqlist)
    {
        if(reqlist->req)
        {
            http_errorReply(reqlist->req, CODE_DEVICE_NO_RESPONSE);
            remove_reqList(reqsession->session->reqList, reqsession->seq);
        }
    }
    return;
}

static void simcom_deviceHandler(struct evhttp_request *req)
{
    char post_data[MSGHTTP_MAX_LEN] = {0};
    evbuffer_copyout(req->input_buffer,post_data,MSGHTTP_MAX_LEN);

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

    cJSON *param = cJSON_GetObjectItem(json, "param");
    if(!param)
    {
        LOG_ERROR("no param in data");
        cJSON_Delete(json);
        http_errorReply(req, CODE_ERROR_CONTENT);
        return;
    }

    cJSON *action = cJSON_GetObjectItem(json, "action");
    if(!action)
    {
        LOG_ERROR("no action in data");
        cJSON_Delete(json);
        http_errorReply(req, CODE_ERROR_CONTENT);
        return;
    }

    char *data = cJSON_PrintUnformatted(param);
    if(!data)
    {
        LOG_ERROR("no memory");
        cJSON_Delete(json);
        http_errorReply(req, CODE_INTERNAL_ERROR);
        return;
    }

    int msgLen = sizeof(MSG_DEVICE_REQ) + strlen(data);
    MSG_DEVICE_REQ *msg = alloc_simcom_msg(CMD_DEVICE, msgLen);
    if(!msg)
    {
        LOG_ERROR("no memory");
        cJSON_Delete(json);
        http_errorReply(req, CODE_INTERNAL_ERROR);
        free(data);
        return;
    }
    msg->header.seq = session->http_seq;
    msg->action = action->valueint;
    strncpy(msg->data, data, strlen(data));

    LOG_HEX(msg, sizeof(MSG_DEVICE_REQ));
    pfn(session->bev, msg, msgLen); //simcom_sendMsg
    session->reqList = insert_reqList(session->reqList, req, session->http_seq++);
    cJSON_Delete(json);
    free(data);
    REQ_EVENT *reqsession = (REQ_EVENT *)malloc(sizeof(REQ_EVENT));
    reqsession->session = session;
    reqsession->seq = session->http_seq - 1;

    //set a timer to response to http if request can't get response from device.
    struct timeval tv = {10, 5000};// X.005 seconds
    timer_newOnce(session->base, &tv, timer_cb, reqsession);

    return;
}

void simcom_http_handler(struct evhttp_request *req, void *arg __attribute__((unused)))
{
    switch(req->type)
    {
        case EVHTTP_REQ_POST:
            if(strstr(req->uri, "/v1/device"))
            {
                simcom_deviceHandler(req);
            }
            break;

        case EVHTTP_REQ_PUT:
        case EVHTTP_REQ_GET:
        case EVHTTP_REQ_DELETE:
        default:
            break;
    }
    return;
}


