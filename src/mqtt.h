/*
 * mqtt.h
 *
 *  Created on: Apr 21, 2015
 *      Author: jk
 */

#ifndef SRC_MQTT_H_
#define SRC_MQTT_H_

#include <mosquitto.h>

struct mosquitto* mqtt_login(const char* id, const char* host, int port,
		void (*on_log)(struct mosquitto *, void *, int, const char *),
		void (*on_connect)(struct mosquitto *, void *, int),
		void (*on_disconnect)(struct mosquitto *, void *, int),
		void (*on_message)(struct mosquitto *, void *, const struct mosquitto_message *),
		void (*on_subscribe)(struct mosquitto *, void *, int, int, const int *),
		void (*on_publish)(struct mosquitto *, void *, int),
		void* ctx);

#endif /* SRC_MQTT_H_ */
