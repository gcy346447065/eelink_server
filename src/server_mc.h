#ifndef _SERVER_MC_
#define _SERVER_MC_

#include <event2/event.h>

struct evconnlistener* server_mc_start(struct event_base*, int);

#endif //_SERVER_MC_
