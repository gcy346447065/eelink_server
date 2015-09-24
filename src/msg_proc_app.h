/*
 * msg_proc_app.h
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */

#ifndef SRC_MSG_PROC_APP_H_
#define SRC_MSG_PROC_APP_H_

#include "object_mc.h"
#include "cb_ctx_mc.h"

void app_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
void app_connect_callback(struct mosquitto *mosq, void *userdata, int result);
void app_disconnect_callback(struct mosquitto *mosq, void *userdata, int rc);
void app_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos);
void app_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str);
void app_publish_callback(struct mosquitto *mosq, void *userdata, int mid);

void app_sendGpsMsg2App(OBJ_MC* obj, void* ctx);
void app_sendRspMsg2App(short cmd, short seq, const void* data, const int len, CB_CTX* ctx);

void app_unsubscribe(struct mosquitto *mosq, void *userdata);

#endif /* SRC_MSG_PROC_APP_H_ */
