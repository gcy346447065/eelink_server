/*
 * msg_proc_http.c
 *
 *  Created on: 2016/11/28
 *      Author: lc
 */
#include <event.h>
#include <evutil.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"
#include "log.h"
#include "cJSON.h"
#include "msg_http.h"
#include "phone_alarm.h"
#include "telephone_http.h"

#define MAX_CLLAER_COUNT 10
static char callerNumber[][TELNUMBER_LENGTH + 1] =
{
    "01053912804",
    "057126883072",
    "051482043270",
    "01053912805",
    "051482043271",
    "057126883073",
    "051482043272",
    "01053912806",
    "051482043273",
    "057126883074",
    "051482043274",
};

static void telephone_deleteTelNumber(struct evhttp_request *req, const char *imeiName)
{
    int rc = db_deleteTelNumber(imeiName);
    if(!rc)
    {
        http_okReply(req);
        return;
    }

    http_errorReply(req, CODE_IMEI_NOT_FOUND);
    return;
}

static void telephone_replaceTelNumber(struct evhttp_request *req, const char *imeiName, const char *telNumber)
{
    int rc = db_replaceTelNumber(imeiName, telNumber);
    if(!rc)
    {
        http_okReply(req);
        return;
    }

    http_errorReply(req, CODE_IMEI_NOT_FOUND);
    return;
}

static void telephone_getTelNumber(struct evhttp_request *req, const char *imeiName)
{
    char telNumber[TELNUMBER_LENGTH + 1] = {0};
    char callNumber[TELNUMBER_LENGTH + 1] = {0};

    cJSON *json = cJSON_CreateObject();
    if(!json)
    {
        LOG_FATAL("failed to alloc memory");
        http_errorReply(req, CODE_INTERNAL_ERROR);
        return;
    }

    int rc = db_getTelNumber(imeiName, telNumber, callNumber);
    if(rc)
    {
        cJSON_AddNumberToObject(json, "code", 101);
    }
    else
    {
        cJSON_AddStringToObject(json, "telephone", telNumber);
    }

    char *msg = cJSON_PrintUnformatted(json);
    if(!msg)
    {
        LOG_FATAL("failed to alloc memory");
        http_errorReply(req, CODE_INTERNAL_ERROR);
    }
    cJSON_Delete(json);

    LOG_DEBUG("%s",msg);

    http_postReply(req, msg);
    free(msg);
    return;
}

static void telephone_callTelNumber(struct evhttp_request *req, const char *imeiName)
{
    char post_data[128] = {0};
    char telNumber[TELNUMBER_LENGTH + 1] = {0};
    char callNumber[TELNUMBER_LENGTH + 1] = {0};

    int rc = db_getTelNumber(imeiName,telNumber, callNumber);
    if(rc != 0)
    {
        if(3 == rc)
        {
            http_errorReply(req, CODE_NO_CONTENT);
            return;
        }
        else
        {
            http_errorReply(req, CODE_IMEI_NOT_FOUND);
        }
    }

    evbuffer_copyout(req->input_buffer,post_data,128);
    cJSON * json= cJSON_Parse(post_data);
    if(!json)
    {
        http_errorReply(req, CODE_ERROR_CONTENT);
        return;
    }
    cJSON * telephone = cJSON_GetObjectItem(json, "caller");
    if(!telephone)
    {
        cJSON_Delete(json);
        http_errorReply(req, CODE_ERROR_CONTENT);
        return;
    }
    unsigned char id = telephone->valueint;
    if(id > MAX_CLLAER_COUNT)
    {
        LOG_ERROR("id > %d, no more caller", MAX_CLLAER_COUNT);
        http_errorReply(req, CODE_RANGE_TOO_LARGE);
        return;
    }
    cJSON_Delete(json);
    LOG_INFO("test call alarm server imei(%s) telNumber(%s) caller(%s)", imeiName, telNumber, callerNumber[id]);
    db_updateCallNumber(imeiName, callerNumber[id]);
    phone_alarmWithCaller(telNumber, callerNumber[id]);

    http_okReply(req);
    return;
}


void http_replyTelephone(struct evhttp_request *req, struct event_base *base __attribute__((unused)))
{
    int rc;
    char imei[IMEI_LENGTH + 1] = {0};
    char telNumber[TELNUMBER_LENGTH + 1] = {0};
    LOG_INFO("%s",req->uri);
    switch(req->type)
    {
        case EVHTTP_REQ_GET:
            rc = sscanf(req->uri, "/v1/telephone/%15s%*s", imei);
            if(rc == 1)
            {
                telephone_getTelNumber(req, imei);
                return;
            }
            break;

        case EVHTTP_REQ_PUT:
            rc = sscanf(req->uri, "/v1/telephone/%15s%*s", imei);
            if(rc == 1)
            {
                telephone_callTelNumber(req, imei);
                return;
            }
            break;
        case EVHTTP_REQ_POST:
            rc = sscanf(req->uri, "/v1/telephone/%15s?telephone=%11s%*s", imei, telNumber);
            if(rc == 2)
            {
                LOG_INFO("open call alarm server imei(%s) telNumber(%s)", imei, telNumber);
                telephone_replaceTelNumber(req, imei, telNumber);
                return;
            }
            rc = sscanf(req->uri, "/v1/telephone/%15s%*s", imei);
            if(rc == 1)
            {
                char post_data[128];
                evbuffer_copyout(req->input_buffer,post_data,128);
                cJSON * json= cJSON_Parse(post_data);
                if(!json)
                {
                    http_errorReply(req, CODE_ERROR_CONTENT);
                    return;
                }
                cJSON * telephone = cJSON_GetObjectItem(json, "telephone");
                if(!telephone)
                {
                    cJSON_Delete(json);
                    http_errorReply(req, CODE_ERROR_CONTENT);
                    return;
                }
                strncpy(telNumber, telephone->valuestring, TELNUMBER_LENGTH);
                cJSON_Delete(json);
                LOG_INFO("open call alarm server imei(%s) telNumber(%s)", imei, telNumber);
                telephone_replaceTelNumber(req, imei, telNumber);
                return;
            }
            break;

        case EVHTTP_REQ_DELETE:
            rc = sscanf(req->uri, "/v1/telephone/%15s%*s", imei);
            if(rc == 1)
            {
                LOG_INFO("close call alarm server imei(%s)", imei);
                telephone_deleteTelNumber(req, imei);
                return;
            }
            break;

         default:
            LOG_ERROR("unkown http type: %d",req->type);
            break;

    }

    http_errorReply(req, CODE_URL_ERR);
    return;
}

void http_replyCall(struct evhttp_request *req, struct event_base *base __attribute__((unused)))
{
    int rc;
    char telNumber[TELNUMBER_LENGTH + 1] = {0};

    LOG_INFO("%s",req->uri);
    switch(req->type)
    {
        case EVHTTP_REQ_GET:
        case EVHTTP_REQ_PUT:
        case EVHTTP_REQ_POST:
        case EVHTTP_REQ_DELETE:
            rc = sscanf(req->uri, "/v1/test/%11s%*s", telNumber);
            if(rc == 1)
            {
                 phone_alarm(telNumber);
                 http_okReply(req);
                 return;
            }
            break;

        default:
            LOG_ERROR("unkown http type: %d",req->type);
            break;
    }

    http_errorReply(req, CODE_URL_ERR);
    return;
}


