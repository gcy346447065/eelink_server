#include <glog/logging.h>

#include "mqtt_top.h"
#include "cJSON.h"

int myMosq::port = 1883;
int myMosq::keepalive = 60;// Basic configuration setup for myMosq class
const char * myMosq::id = "client_test";
const char * myMosq::host = "localhost";
const char * myMosq::topic = "dev2app/+/gps";

bool myMosq::send_message(const char *imei, GPS *gps)
{
    char topic[50] = {0};
    snprintf(topic, 50, "dev2app/%s/gps", imei);

    cJSON * root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", gps->timestamp);
    cJSON_AddNumberToObject(root, "isGPSlocated", 1);
    cJSON_AddNumberToObject(root, "lat", gps->latitude);
    cJSON_AddNumberToObject(root, "lng", gps->longitude);
    cJSON_AddNumberToObject(root, "speed", gps->speed);
    cJSON_AddNumberToObject(root, "course", gps->course);

    char *_message = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    int ret = publish(NULL,this->topic, strlen(_message), _message, 2, false);
    free(_message);
    return ( ret == MOSQ_ERR_SUCCESS );
}

void myMosq::on_connect(int rc)
{
    if ( rc == 0 ) {
        LOG(INFO) << ">> myMosq - connected with server";
    } else {
        LOG(ERROR) << ">> myMosq - Impossible to connect with server(" << rc << ")";;
    }
}

void myMosq::on_disconnect(int rc) {
    LOG(ERROR) << ">> myMosq - disconnection(" << rc << ")";
}

void myMosq::on_publish(int mid)
{
    LOG(INFO) << ">> myMosq - Message (" << mid << ") succeed to be published ";
}

