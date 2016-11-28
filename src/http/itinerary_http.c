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
#include "itinerary_http.h"

static int one_Itinerary(int starttime, double startlat, double startlon, int endtime, double endlat, double endlon, short miles, void *userdata)
{
    if(!userdata)
    {
        LOG_ERROR("userdata is null.");
        return 0;
    }

	cJSON *iItitnerary = cJSON_CreateObject();
	cJSON *iStart = cJSON_CreateObject();
	cJSON *iEnd = cJSON_CreateObject();

	cJSON_AddNumberToObject(iStart, "timestamp", starttime);
	cJSON_AddNumberToObject(iStart, "lat", startlat);
	cJSON_AddNumberToObject(iStart, "lon", startlon);
	cJSON_AddNumberToObject(iEnd, "timestamp", endtime);
	cJSON_AddNumberToObject(iEnd, "lat", endlat);
	cJSON_AddNumberToObject(iEnd, "lon", endlon);

	cJSON_AddItemToObject(iItitnerary, "start", iStart);
	cJSON_AddItemToObject(iItitnerary, "end", iEnd);
	cJSON_AddNumberToObject(iItitnerary, "miles", miles);

	cJSON_AddItemToArray((cJSON *)userdata, iItitnerary);

    return 0;
}

static void http_getItinerary(struct evhttp_request *req, const char *imeiName, int starttime, int endtime)
{
    cJSON *json = cJSON_CreateObject();
    if(!json)
    {
        LOG_FATAL("failed to alloc memory");
        http_errorMsg(req);
        return;
    }

    cJSON *itinerary_Array = cJSON_CreateArray();
    if(!itinerary_Array)
    {
        LOG_FATAL("failed to alloc memory");
        cJSON_Delete(json);
        http_errorMsg(req);
        return;
    }

    int rc = db_getItinerary(imeiName, starttime, endtime,(void *)one_Itinerary,itinerary_Array);
    int num = cJSON_GetArraySize(itinerary_Array);

    LOG_INFO("there are %d itinerary", num);

    if(rc)
    {
        LOG_WARN("no database itinerary_%s", imeiName);
        cJSON_AddNumberToObject(json, "code", 101);
    }
    else if(num == 0)
    {
        LOG_INFO("%s no data bettween %d and %d", imeiName, starttime, endtime);
        cJSON_AddNumberToObject(json, "code", 102);
    }
    else
    {
        cJSON_AddItemToObject(json, "itinerary", itinerary_Array);
    }

    char *msg = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    LOG_DEBUG("%s",msg);

    http_rspMsg(req, msg);
    free(msg);
    return;
}

void http_replyItinerary(struct evhttp_request *req)
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
            rc = sscanf(req->uri, "/v1/itinerary/%15s?start=%d&end=%d%*s", imei, &start, &end);
            if(rc == 3)
            {
                http_getItinerary(req, imei, start, end);
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


