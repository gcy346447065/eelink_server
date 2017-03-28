#include <math.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <mosquitto.h>
#include <event2/event.h>
#include <event2/listener.h>

#include "db.h"
#include "log.h"
#include "mqtt.h"
#include "cJSON.h"
#include "itinerary_object.h"

#define EARTH_RADIUS 6378137 //radius of our earth unit :  m
#define PI 3.141592653
#define DEG2RAD(d) (d * PI / 180.f)//degree to radian

static long double get_distancebetweenGPS(float pre_gpsLat, float pre_gpsLon,float new_gpsLat,float new_gpsLon)
{
    long double radLat1 = DEG2RAD(pre_gpsLat);
    long double radLat2 = DEG2RAD(new_gpsLat);
    long double a = radLat1 - radLat2;
    long double b = DEG2RAD(pre_gpsLon) - DEG2RAD(new_gpsLon);

    long double s = 2 * asin(sqrt(sin(a/2)*sin(a/2)+cos(radLat1)*cos(radLat2)*sin(b/2)*sin(b/2)));

    s = s * EARTH_RADIUS;
    return s ;
}


static void one_minute_loop_cb(evutil_socket_t fd __attribute__((unused)), short what __attribute__((unused)), void *arg)
{
    LOG_INFO("one minutes timer for ItieraryJudge_cb");

    itineraryObj_table_ItieraryJudge((void *)db_saveiItinerary);
    return;
}

static void getImeiFromTopic(const char* topic, char* IMEI)
{
    const char* pStart = &topic[strlen("app2dev/")];
    const char* pEnd = strstr(pStart, "/");

    if (pEnd - pStart > IMEI_LENGTH)
    {
        LOG_ERROR("app2dev: imei length too long");
        return;
    }

    strncpy(IMEI, pStart, pEnd - pStart);

    return;
}


static int itinerary_handleAppGPSMsg(const char* topic, const char* data, const int len __attribute__((unused)))
{
    if (!data)
    {
        LOG_FATAL("internal error: payload null");
        return -1;
    }
    LOG_DEBUG("topic = %s, payload = %s", topic, data);

    char strIMEI[IMEI_LENGTH + 1] = {0};
    getImeiFromTopic(topic, strIMEI);

    ITINERARY_OBJECT* obj = itineraryObj_get(strIMEI);
    if (!obj)
    {
        obj = itineraryObj_new();
        if(!obj)
        {
            LOG_FATAL("internal error: payload null");
            return -1;
        }
        strncpy(obj->IMEI, strIMEI, IMEI_LENGTH);
        itineraryObj_add(obj);
    }

    cJSON *root = cJSON_Parse(data);
    if (!root)
    {
        LOG_ERROR("data is not json: %s", cJSON_GetErrorPtr());
        return -1;
    }

    cJSON* json_lat = cJSON_GetObjectItem(root, "lat");
    if (!json_lat)
    {
        LOG_ERROR("no lat item");
        cJSON_Delete(root);
        return -1;
    }

    cJSON* json_lng = cJSON_GetObjectItem(root, "lng");
    if (!json_lng)
    {
        LOG_ERROR("no lng item");
        cJSON_Delete(root);
        return -1;
    }

    cJSON* json_timestamp = cJSON_GetObjectItem(root, "timestamp");
    if (!json_timestamp)
    {
        LOG_ERROR("no timestamp item");
        cJSON_Delete(root);
        return -1;
    }

    cJSON_Delete(root);

    float lat_pre = obj->lat;
    float lon_pre = obj->lon;
    int timestamp_pre = obj->timestamp;
    obj->lat = json_lat->valuedouble;
    obj->lon = json_lng->valuedouble;
    obj->timestamp = json_timestamp->valueint;

    double miles = get_distancebetweenGPS(lat_pre, lon_pre, obj->lat, obj->lon) + 0.5;
    if(obj->isStarted)
    { /* if itinerary has started, add the miles instantly */
        obj->itineray += (int)miles;
        db_updateItinerary(obj->IMEI, (int)miles);
    }
    else
    { /* if itinerary has not started, judge start new itinerary */
        obj->itineray = 0;
        obj->isStarted = 1;
        obj->startlat = obj->lat;
        obj->startlon = obj->lon;
        obj->starttime = obj->timestamp;
    }

    obj->timecount = 0;//every GPS comes, set the count as 0, when it reach 5, one itinerary generats
    return 0;
}

int main(int argc, char *argv[])
{
    int rc = log_init("../conf/itinerary_log.conf");
    if (rc)
    {
        LOG_FATAL("log initial failed: %d", rc);
    	return rc;
    }

    rc = db_initial();
    if(rc)
    {
        LOG_FATAL("connect to mysql failed");
        return -1;
    }

    struct event_base *base = event_base_new();
    if (!base)
    {
        LOG_FATAL("event base new failed");
        return 1;
    }

    rc = mosquitto_lib_init();
    if (rc != MOSQ_ERR_SUCCESS)
    {
    	LOG_FATAL("mosquitto lib initial failed: %d", rc);
    	return -1;
    }

    MQTT_ARG mqtt_arg ={ base, itinerary_handleAppGPSMsg };
    mqtt_initial(&mqtt_arg);

    rc = mqtt_subscribe_allGPS();
    if (rc < 0)
    {
        LOG_FATAL("mqtt subscribe failed: %d", rc);
    	return rc;
    }

    struct timeval one_min = { 60, 0 };
    timer_newLoop(base, &one_min, one_minute_loop_cb, NULL);

    event_base_dispatch(base);

    event_base_free(base);

    mqtt_cleanup();

    mosquitto_lib_cleanup();

    LOG_FATAL("mqtt subscribe failed: %d", rc);

	return 0;
}

