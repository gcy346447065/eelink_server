/*
 * cb_ctx_mc.h
 *
 *  Created on: Apr 19, 2015
 *      Author: jk
 */

#ifndef SRC_CB_CTX_MC_H_
#define SRC_CB_CTX_MC_H_

#include <stdio.h>
#include <curl/curl.h>
#include <event2/bufferevent.h>

/**
* container_of - cast a member of a structure out to the containing structure
* @ptr:     the pointer to the member.
* @type:     the type of the container struct this is embedded in.
* @member:     the name of the member within the struct.
*
*/
#define container_of(ptr, type, member) ({             \
         const typeof( ((type *)0)->member ) *__mptr = (ptr);     \
         (type *)( (char *)__mptr - offsetof(type,member) );})

typedef void (*msg_send)(struct bufferevent* bev, const void* buf, size_t n);

typedef struct
{
	struct event_base* base;
	struct bufferevent* bev;
	CURL *curlOfLeancloud;
	CURL *curlOfYeelink;
	void* obj;
	msg_send pSendMsg;
	int cur_status;
}CB_CTX;

#endif /* SRC_CB_CTX_MC_H_ */
