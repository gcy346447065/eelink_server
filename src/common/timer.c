/*
 * timer.c
 *
 *  Created on: Sep 27, 2015
 *      Author: jk
 */
#include <stdio.h>
#include <event2/event.h>

#include "log.h"

void timer_mqtt_fn(evutil_socket_t fd, short a, void * arg)
{
    LOG_INFO("reconnect MQTT");
}


void timer_reconnectMQTT(struct event_base *base)
{
    struct timeval ten_sec = { 10, 0 };

    struct event *ev = evtimer_new(base, timer_mqtt_fn, NULL);

    evtimer_add(ev, &ten_sec);
}
