/*
 * slb.h
 *
 *  Created on: 2015Äê6ÔÂ5ÈÕ
 *      Author: jk
 */

#ifndef SRC_SLB_H_
#define SRC_SLB_H_

#include <event2/event.h>

struct evconnlistener* slb_start(struct event_base*, int);

#endif /* SRC_SLB_H_ */
