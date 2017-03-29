#include <netinet/in.h>
#include <string.h>

#include "cJSON.h"
#include "logger.h"
#include "mqtt_top.h"

int myMosq::port = 1883;
int myMosq::keepalive = 60;// Basic configuration setup for myMosq class
const char * myMosq::id = "gps_server";
const char * myMosq::host = "localhost";

bool myMosq::send_message(const char *imei, GPS *gps)
{
    char topic[50] = {0};
    snprintf(topic, 50, "dev2app/%s/gps", imei);

    cJSON * root = cJSON_CreateObject();
    if(!root)
    {
        LOG_FATAL << "no enough memory";
        return false;
    }
    cJSON_AddNumberToObject(root, "timestamp", ntohl(gps->timestamp));
    cJSON_AddNumberToObject(root, "isGPSlocated", 1);
    cJSON_AddNumberToObject(root, "lat", gps->latitude);
    cJSON_AddNumberToObject(root, "lng", gps->longitude);
    cJSON_AddNumberToObject(root, "speed", gps->speed);
    cJSON_AddNumberToObject(root, "course", ntohs(gps->course));

    char *_message = cJSON_PrintUnformatted(root);
    if(!_message)
    {
        LOG_FATAL << "no enough memory";
        cJSON_Delete(root);
        return false;
    }
    cJSON_Delete(root);

    int ret = publish(NULL, topic, strlen(_message), _message, 2, false);

    free(_message);

    return ( ret == MOSQ_ERR_SUCCESS );
}

void myMosq::on_connect(int rc)
{
    if ( rc == 0 ) {
        LOG_INFO << "myMosq connected with server";
    } else {
        LOG_ERROR << "myMosq Impossible to connect with server" << rc;
    }
}

void myMosq::on_disconnect(int rc) {
    LOG_ERROR << "myMosq disconnection with server" << rc;
}

void myMosq::on_publish(int mid)
{
    LOG_INFO << "myMosq - Message " << mid <<" succeed to be published ";
}

