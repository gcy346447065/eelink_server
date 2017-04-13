/*
 * mqtt.h
 *
 *  Created on: Apr 21, 2015
 *      Author: jk
 */

#ifndef SRC_MQTT_H_
#define SRC_MQTT_H_

typedef int (*APP_MSG_HANDLER)(const char* topic, const char* data, const int len);

typedef struct
{
    struct event_base *base;
    APP_MSG_HANDLER app_msg_handler;
}MQTT_ARG;

void mqtt_initial(MQTT_ARG* mqtt_arg);
void mqtt_cleanup();

void mqtt_publish(const char *topic, const void *payload, size_t payloadlen);
void mqtt_subscribe(const char *imei);
void mqtt_unsubscribe(const char *imei);

int mqtt_subscribe_allGPS(void);
int mqtt_unsubscribe_allGPS(void);

#endif /* SRC_MQTT_H_ */
