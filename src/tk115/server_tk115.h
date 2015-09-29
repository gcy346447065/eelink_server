#ifndef _SERVER_TK115_
#define _SERVER_TK115_

#include <event2/event.h>

struct evconnlistener*server_start(struct event_base *, int);

#endif //_SERVER_TK115_
