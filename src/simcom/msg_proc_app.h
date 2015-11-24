/*
 * msg_proc_app.h
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */

#ifndef SRC_MSG_PROC_APP_H_
#define SRC_MSG_PROC_APP_H_

int app_handleApp2devMsg(const char* topic, const char* data, const int len, void* userdata);

void app_sendCmdRsp2App(int cmd, int result, const char *strIMEI);
void app_sendGpsMsg2App(void *session);
void app_send433Msg2App(int timestamp, int intensity, void *session);
void app_sendAlarmMsg2App(unsigned char type, const char *msg, void *session);
void app_sendFenceGetRspMsg2App(int cmd, int result, int state, void *session);
void app_sendAutolockMsg2App(int timestamp, int lock, void * session);

#endif /* SRC_MSG_PROC_APP_H_ */
