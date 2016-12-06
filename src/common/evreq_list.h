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

int insert_reqList(REQLIST *reqList, struct evhttp_request *req, unsigned char seq);
void remove_reqList(REQLIST *reqList, unsigned char seq);
REQLIST *init_reqList(REQLIST *reqList);
int distruct_reqList(REQLIST *reqList);
REQLIST *find_reqList(REQLIST *reqList, unsigned char seq);

#endif/*    SRC_EVREQ_LIST_H_   */

