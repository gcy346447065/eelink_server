/*
 * http.h
 *
 *  Created on: 2016/12/06
 *      Author: lc
 */

#ifndef SRC_EVREQ_LIST_H_
#define SRC_EVREQ_LIST_H_

#include <evhttp.h>

typedef struct NODE
{
    unsigned char seq;
    struct evhttp_request *req;
    struct NODE *next;
} REQLIST;

REQLIST *init_reqList(void);
REQLIST *insert_reqList(REQLIST *reqList, struct evhttp_request *req, unsigned char seq);
REQLIST *remove_reqList(REQLIST *reqList, unsigned char seq);
REQLIST *find_reqList(REQLIST *reqList, unsigned char seq);
REQLIST *distruct_reqList(REQLIST *reqList);

#endif/*    SRC_EVREQ_LIST_H_   */

