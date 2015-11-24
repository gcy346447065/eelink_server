/*
 * server_sync.h
 *
 *  Created on: 2015/10/30
 *      Author: jk
 */

#ifndef SRC_SERVER_SYNC_H_
#define SRC_SERVER_SYNC_H_

#include <event2/event.h>

struct evconnlistener* server_sync(struct event_base*, int);

#endif /* SRC_SERVER_SYNC_H_ */
