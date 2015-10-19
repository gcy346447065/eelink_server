/*
 * cgi2gps.c
 *
 *  Created on: Oct 19, 2015
 *      Author: msj
 */
#include <curl/curl.h>
#include <stdio.h>

#include "cJSON.h"
#include "macro.h"
#include "log.h"

#define MAX_LEN 400
#define ADD2RXL 110

typedef struct
{
   float lat;
   float lon;
}GPS;

static void make_url(CGI_MC cell[], int cellNo, char *url)
{
    char *current = url;
    int step = snprintf(current, MAX_LEN, "http://minigps.net/as?x=%x-%x", cell[0].mcc, cell[0].mnc);
    int i;
    for(i = 0, current += step; i < cellNo; ++i,current += step)
    {
        step = snprintf(current, MAX_LEN, "-%x-%x-%x", (unsigned short)cell[i].lac, (unsigned short)cell[i].ci, cell[i].rxl + ADD2RXL);
    }
    //TODO:&p&mt&ta&needaddress
    snprintf(current, MAX_LEN, "&ta=1&needaddress=0");
    return;
}

static size_t handleGetGps(void *buffer, size_t size, size_t nmemb, void *userp)
{
    GPS *gps = (GPS *)userp;

    cJSON *gpsMsg = cJSON_Parse((const char *)buffer);
    if(!gpsMsg)
    {
        LOG_ERROR("gps message format not json: %s", cJSON_GetErrorPtr());
        return 0;
    }
    gps->lat = (cJSON_GetObjectItem(gpsMsg, "lat"))->valuedouble;
    gps->lon = (cJSON_GetObjectItem(gpsMsg, "lon"))->valuedouble;
    return size * nmemb;
}

int cgi2gps(CGI_MC cell[], int cellNo, float *lat, float *lon)
{
    GPS gps;
    char url[MAX_LEN];
    make_url(cell, cellNo, url);
    CURL *curl = curl_easy_init();
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handleGetGps);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&gps);
    
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if(res != CURLE_OK)
    {
        return -1;
    }
    else
    {
        *lat = gps.lat;
        *lon = gps.lon;
        return 0;
    }
}
