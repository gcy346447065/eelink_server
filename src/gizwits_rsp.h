/*
 * gizwits_rsp.h
 *
 *  Created on: Apr 21, 2015
 *      Author: jk
 */

#ifndef SRC_GIZWITS_RSP_H_
#define SRC_GIZWITS_RSP_H_

#include "cb_ctx_mc.h"

int mc_register_rsp(int response_code, const char* msg, CB_CTX* ctx);
int mc_provision_rsp(int response_code, const char* msg, CB_CTX* ctx);

int mqtt_app2dev(const char* topic, const char* data, const int len, void* userdata);
int mqtt_ser2cli_res(const char* topic, const char* data, const int len, void* userdata);
int mqtt_ser2cli_noti(const char* topic, const char* data, const int len, void* userdata);
#endif /* SRC_GIZWITS_RSP_H_ */
