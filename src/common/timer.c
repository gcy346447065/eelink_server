/*
 * timer.c
 *
 *  Created on: Sep 27, 2015
 *      Author: jk
 */
#include <stdio.h>
#include <stdlib.h>

#include "timer.h"
#include "log.h"

struct event* timer_newLoop(struct event_base *base, const struct timeval *timeout, event_callback_fn cb, void *arg)
{
    struct event *ev = event_new(base, -1, EV_PERSIST, cb, arg);
    if (!ev)
    {
        LOG_FATAL("in-sufficent memroy");
        exit(0);
    }
    else
    {
        LOG_INFO("new loop timer of period %ds", timeout->tv_sec);
    }
    evtimer_add(ev, timeout);

    return ev;
}

struct event* timer_newOnce(struct event_base *base, const struct timeval *timeout, event_callback_fn cb, void *arg)
{
    struct event *ev = evtimer_new(base, cb, arg);
    if (!ev)
    {
        LOG_FATAL("in-sufficent memroy");
        exit(0);
    }
    else
    {
        LOG_INFO("new one-shot timer of period %ds", timeout->tv_sec);
    }
    evtimer_add(ev, timeout);

    return ev;
}

void timer_react(struct event *ev, struct timeval *timeout)
{
    LOG_INFO("react a timer of period %ds", timeout->tv_sec);

    evtimer_add(ev, timeout);
}