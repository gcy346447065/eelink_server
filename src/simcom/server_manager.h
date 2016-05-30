/*
 * server_simcom.h
 *
 *  Created on: 2015/6/25
 *      Author: jk
 */

#ifndef SRC_SERVER_MANAGER_H_
#define SRC_SERVER_MANAGER_H_

#include <event2/event.h>

struct evconnlistener *server_manager(struct event_base* base, int port);

#endif /* SRC_SERVER_MANAGER_H_ */
