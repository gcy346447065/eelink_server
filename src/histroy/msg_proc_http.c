/*
 * msg_proc_http.c
 *
 *  Created on: 2016/10/13
 *      Author: lc
 */
#include <stdio.h>

#include "msg_proc_http.h"

#include "cJSON.h"
#include "log.h"
#include "malloc.h"
#include "db.h"

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

char *history_getGPS(const char *imeiName, int starttime, int endtime)
{
    cJSON *rsp = cJSON_CreateObject();
    if(!rsp)
    {
        LOG_FATAL("failed to alloc memory");
        return NULL;
    }

    cJSON *gps_Array = cJSON_CreateArray();
    if(!gps_Array)
    {
        LOG_FATAL("failed to alloc memory");
        cJSON_Delete(rsp);
        return NULL;
    }


    int rc = db_getGPS(imeiName, starttime, endtime, one_GPS, gps_Array);
    int num = cJSON_GetArraySize(gps_Array);

    LOG_INFO("there are %d gps", num);

    if(rc)
    {
        LOG_ERROR("no database gps_%s", imeiName);
        cJSON_AddNumberToObject(rsp, "code", 101);
    }
    else if(num == 0)
    {
        LOG_INFO("%s no data bettween %d and %d", imeiName, starttime, endtime);
        cJSON_AddNumberToObject(rsp, "code", 102);
    }
    else
    {
        cJSON_AddItemToObject(rsp, "gps", gps_Array);
    }
    char *json = cJSON_PrintUnformatted(rsp);
    LOG_DEBUG("%s",json);
    cJSON_Delete(rsp);
    return json;
}


char *history_getItinerary(const char *imeiName, int starttime, int endtime)
{
    cJSON *rsp = cJSON_CreateObject();
    if(!rsp)
    {
        LOG_FATAL("failed to alloc memory");
        return NULL;
    }

    cJSON *itinerary_Array = cJSON_CreateArray();
    if(!itinerary_Array)
    {
        LOG_FATAL("failed to alloc memory");
        cJSON_Delete(rsp);
        return NULL;
    }

    int rc = db_getItinerary(imeiName, starttime, endtime,one_Itinerary,itinerary_Array);
    int num = cJSON_GetArraySize(itinerary_Array);

    LOG_INFO("there are %d itinerary", num);

    if(rc)
    {
        LOG_ERROR("no database itinerary_%s", imeiName);
        cJSON_AddNumberToObject(rsp, "code", 101);
    }
    else if(num == 0)
    {
        LOG_INFO("%s no data bettween %d and %d", imeiName, starttime, endtime);
        cJSON_AddNumberToObject(rsp, "code", 102);
    }
    else
    {
        cJSON_AddItemToObject(rsp, "itinerary", itinerary_Array);
    }
    char *json = cJSON_PrintUnformatted(rsp);
    LOG_DEBUG("%s",json);
    cJSON_Delete(rsp);
    return json;
}

void history_freeMsg(char *msg)
{
    free(msg);
}



