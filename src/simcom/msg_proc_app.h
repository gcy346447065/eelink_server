/*
 * msg_proc_app.h
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */

#ifndef SRC_MSG_PROC_APP_H_
#define SRC_MSG_PROC_APP_H_

int app_handleApp2devMsg(const char* topic, const char* data, const int len, void* userdata);

void app_sendGpsMsg2App(void *session);
void app_send433Msg2App(int timestamp, int intensity, void * session);
void app_sendCmdMsg2App(int cmd, int result, char *state, void *session);

char* app_getCmdString(int cmd);

#endif /* SRC_MSG_PROC_APP_H_ */
