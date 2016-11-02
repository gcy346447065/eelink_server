/*
 * msg_proc_http.c
 *
 *  Created on: 2016/10/13
 *      Author: lc
 */
#include <stdio.h>

#include "msg_proc_http.h"
#include "protocol_history.h"

#include "cJSON.h"
#include "log.h"
#include "malloc.h"
#include "db.h"

char *history_getGPS(const char *imeiName, int starttime, int endtime)
{
    cJSON *gps_Array = NULL;
    cJSON *rsp = cJSON_CreateObject();;
    HISTORY_GPS_RSP *gps = (HISTORY_GPS_RSP *)db_getGPS(imeiName, starttime, endtime);

    if(!gps)
    {
        LOG_ERROR("no database gps_%s", imeiName);
        cJSON_AddNumberToObject(rsp, "code", 101);
    }
    else if(gps->num == 0)
    {
        LOG_INFO("%s no data bettween %d and %d", imeiName, starttime, endtime);
        cJSON_AddNumberToObject(rsp, "code", 102);
    }
    else
    {
        cJSON *iGps = NULL;
        gps_Array = cJSON_CreateArray();
        LOG_INFO("%s get gps number:%d", imeiName, gps->num);
        for(int i = 0; i < gps->num;i++)
        {
            iGps = cJSON_CreateObject();
            cJSON_AddNumberToObject(iGps, "timestamp", gps->gps[i].timestamp);
            cJSON_AddNumberToObject(iGps, "lat", gps->gps[i].latitude);
            cJSON_AddNumberToObject(iGps, "lon", gps->gps[i].longitude);
            cJSON_AddNumberToObject(iGps, "speed", gps->gps[i].speed);
            cJSON_AddNumberToObject(iGps, "course", gps->gps[i].course);
            cJSON_AddItemToArray(gps_Array, iGps);
        }
        cJSON_AddItemToObject(rsp, "gps", gps_Array);
    }
    char *json = cJSON_PrintUnformatted(rsp);
    LOG_DEBUG("%s",json);
    cJSON_Delete(rsp);
    free(gps);
    return json;
}

char *history_getItinerary(const char *imeiName, int starttime, int endtime)
{
    cJSON *itinerary_Array = NULL;
    cJSON *rsp = cJSON_CreateObject();;
    HISTORY_GPS_RSP *gps = (HISTORY_GPS_RSP *)db_getGPS(imeiName, starttime, endtime);

    if(!gps)
    {
        LOG_ERROR("no database gps_%s", imeiName);
        cJSON_AddNumberToObject(rsp, "code", 101);
    }
    else if(gps->num == 0)
    {
        LOG_INFO("%s no data bettween %d and %d", imeiName, starttime, endtime);
        cJSON_AddNumberToObject(rsp, "code", 102);
    }
    else
    {
        cJSON *iGps = NULL;
        itinerary_Array = cJSON_CreateArray();
        LOG_INFO("%s get gps number:%d", imeiName, gps->num);
        for(int i = 0; i < gps->num;i++)
        {
            iGps = cJSON_CreateObject();
            cJSON_AddNumberToObject(iGps, "timestamp", gps->gps[i].timestamp);
            cJSON_AddNumberToObject(iGps, "lat", gps->gps[i].latitude);
            cJSON_AddNumberToObject(iGps, "lon", gps->gps[i].longitude);
            cJSON_AddNumberToObject(iGps, "speed", gps->gps[i].speed);
            cJSON_AddNumberToObject(iGps, "course", gps->gps[i].course);
            cJSON_AddItemToArray(itinerary_Array, iGps);
        }
        cJSON_AddItemToObject(rsp, "itinerary", itinerary_Array);
    }
    char *json = cJSON_PrintUnformatted(rsp);
    LOG_DEBUG("%s",json);
    cJSON_Delete(rsp);
    free(gps);
    return json;
}

void history_freeMsg(char *msg)
{
    free(msg);
}



