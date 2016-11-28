/*
 * msg_proc_http.c
 *
 *  Created on: 2016/10/13
 *      Author: lc
 */
#include <stdlib.h>
#include <string.h>

#include "db.h"
#include "log.h"
#include "cJSON.h"
#include "msg_http.h"
#include "gps_http.h"

static int one_GPS(int timestamp, double latitude, double longitude, char speed, short course, void *userdata)
{
    if(!userdata)
    {
        LOG_ERROR("userdata is null.");
        return 0;
    }

    cJSON *iGps = cJSON_CreateObject();

    cJSON_AddNumberToObject(iGps, "timestamp", timestamp);
    cJSON_AddNumberToObject(iGps, "lat", latitude);
    cJSON_AddNumberToObject(iGps, "lon", longitude);
    cJSON_AddNumberToObject(iGps, "speed", speed);
    cJSON_AddNumberToObject(iGps, "course", course);

    cJSON_AddItemToArray((cJSON *)userdata, iGps);

    return 0;
}

static void http_getGPS(struct evhttp_request *req, const char *imeiName, int starttime, int endtime)
{
    cJSON *json = cJSON_CreateObject();
    if(!json)
    {
        LOG_FATAL("failed to alloc memory");
        http_errorMsg(req);
        return;
    }

    cJSON *gps_Array = cJSON_CreateArray();
    if(!gps_Array)
    {
        LOG_FATAL("failed to alloc memory");
        cJSON_Delete(json);
        http_errorMsg(req);
        return;
    }

    int rc = db_getGPS(imeiName, starttime, endtime, (void *)one_GPS, gps_Array);
    int num = cJSON_GetArraySize(gps_Array);

    LOG_INFO("there are %d gps", num);

    if(rc)
    {
        LOG_WARN("no database gps_%s", imeiName);
        cJSON_AddNumberToObject(json, "code", 101);
    }
    else if(num == 0)
    {
        LOG_INFO("%s no data bettween %d and %d", imeiName, starttime, endtime);
        cJSON_AddNumberToObject(json, "code", 102);
    }
    else
    {
        cJSON_AddItemToObject(json, "gps", gps_Array);
    }
    char *msg = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    LOG_DEBUG("%s",msg);
    http_rspMsg(req, msg);

    free(msg);
    return;
}

void http_replyGPS(struct evhttp_request *req)
{
    int rc;
    int start = 0, end = 0;
    char imei[IMEI_LENGTH + 1] = {0};

    LOG_INFO("%s",req->uri);
    switch(req->type)
    {
        case EVHTTP_REQ_GET:
        case EVHTTP_REQ_PUT:
        case EVHTTP_REQ_POST:
        case EVHTTP_REQ_DELETE:
            rc = sscanf(req->uri, "/v1/history/%15s?start=%d&end=%d%*s", imei, &start, &end);
            if(rc == 3)
            {
                http_getGPS(req, imei, start, end);
                return;
            }
            break;

        default:
            LOG_ERROR("unkown http type: %d",req->type);
            break;
    }

    http_errorMsg(req);
    return;
}

