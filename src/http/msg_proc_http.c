/*
 * msg_proc_http.c
 *
 *  Created on: 2016/10/13
 *      Author: lc
 */
#include <stdlib.h>
#include <string.h>

#include "msg_proc_http.h"
#include "msg_http.h"
#include "cJSON.h"
#include "log.h"
#include "malloc.h"
#include "db.h"
#include "phone_alarm.h"

/*
enum evhttp_cmd_type
{
  EVHTTP_REQ_GET,
  EVHTTP_REQ_POST,
  EVHTTP_REQ_HEAD,
  EVHTTP_REQ_PUT,
  EVHTTP_REQ_DELETE
}
*/

//char *post_data = (char *) EVBUFFER_DATA(req->input_buffer);

#define MAX_CMD_LENGTH 32
typedef void (*MSG_PROC)(struct evhttp_request *req);
typedef struct
{
	const char cmd[MAX_CMD_LENGTH];
	MSG_PROC pfn;
} MSG_PROC_MAP;

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

static void http_replyGPS(struct evhttp_request *req)
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

static int one_Itinerary(int starttime, double startlat, double startlon, int endtime, double endlat, double endlon, int miles, void *userdata)
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

static void http_replyItinerary(struct evhttp_request *req)
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

static void telephone_deleteTelNumber(struct evhttp_request *req, const char *imeiName)
{
    int rc = db_deleteTelNumber(imeiName);
    if(!rc)
    {
        http_okMsg(req);
        return;
    }

    http_errorMsg(req);
    return;
}

static void telephone_replaceTelNumber(struct evhttp_request *req, const char *imeiName, const char *telNumber)
{
    int rc = db_replaceTelNumber(imeiName, telNumber);
    if(!rc)
    {
        http_okMsg(req);
        return;
    }

    http_errorMsg(req);
    return;
}

static void telephone_getTelNumber(struct evhttp_request *req, const char *imeiName)
{
    char telNumber[TELNUMBER_LENGTH + 1] = {0};

    cJSON *json = cJSON_CreateObject();
    if(!json)
    {
        LOG_FATAL("failed to alloc memory");
        http_errorMsg(req);
        return;
    }

    int rc = db_getTelNumber(imeiName, telNumber);
    if(rc)
    {
        cJSON_AddNumberToObject(json, "code", 101);
    }
    else
    {
        cJSON_AddStringToObject(json, "telephone", telNumber);
    }

    char *msg = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    LOG_DEBUG("%s",msg);

    http_rspMsg(req, msg);
    free(msg);
    return;
}

static void http_replyTelephone(struct evhttp_request *req)
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
        case EVHTTP_REQ_POST:
            rc = sscanf(req->uri, "/v1/telephone/%15s?telephone=%11s%*s", imei, telNumber);
            if(rc == 2)
            {
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

    http_errorMsg(req);
    return;
}

static void http_replyCall(struct evhttp_request *req)
{
    int rc;
    char number[TELNUMBER_LENGTH + 1] = {0};

    LOG_INFO("%s",req->uri);
    switch(req->type)
    {
        case EVHTTP_REQ_GET:
        case EVHTTP_REQ_PUT:
        case EVHTTP_REQ_POST:
        case EVHTTP_REQ_DELETE:
            rc = sscanf(req->uri, "/v1/test/%11s%*s", number);
            if(rc == 1)
            {
                 phone_alarm(number);
                 http_okMsg(req);
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

static MSG_PROC_MAP msgProcs[] =
{
	{"/v1/history",     http_replyGPS},
    {"/v1/itinerary",   http_replyItinerary},
	{"/v1/telephone",   http_replyTelephone},
	{"/v1/test",        http_replyCall},
};

void httpd_handler(struct evhttp_request *req, void *arg __attribute__((unused)))
{
    for(size_t i = 0; i < sizeof(msgProcs)/sizeof(msgProcs[0]); i++)
    {
        if(strstr(req->uri, msgProcs[i].cmd))
        {
            MSG_PROC pfn = msgProcs[i].pfn;
            if (pfn)
            {
                pfn(req);
                return;
            }
        }
    }
}


