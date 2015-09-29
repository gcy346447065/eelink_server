/*
 * server_simcom.h
 *
 *  Created on: 2015/6/25
 *      Author: jk
 */

#ifndef SRC_SERVER_SIMCOM_H_
#define SRC_SERVER_SIMCOM_H_

#include <event2/event.h>

struct evconnlistener* server_simcom(struct event_base*, int);

#endif /* SRC_SERVER_SIMCOM_H_ */
