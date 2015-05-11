/*
 * mqtt.h
 *
 *  Created on: Apr 21, 2015
 *      Author: jk
 */

#ifndef SRC_MQTT_H_
#define SRC_MQTT_H_

#include <mosquitto.h>

struct mosquitto* mqtt_login(const char* id, const char* host, int port, void* ctx);

#endif /* SRC_MQTT_H_ */
