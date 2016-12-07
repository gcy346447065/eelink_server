/*
 * evreq_list.c
 *
 *  Created on: 2016/12/06
 *      Author: lc
 */
#include <string.h>
#include <malloc.h>

#include "evreq_list.h"
#include "msg_http.h"
#include "log.h"

/*      initail reqList as NULL      */
REQLIST *init_reqList(void){return NULL;}

/*   insert into reqList what req and seq needed, it doesnt matter if reqList is null, and always return the head*/
REQLIST *insert_reqList(REQLIST *reqList, struct evhttp_request *req, unsigned char seq)
{
    REQLIST *ptmp = reqList;
    REQLIST *p = (REQLIST *)malloc(sizeof(REQLIST));
    if(!p)
    {
        LOG_ERROR("create req failed");
        return NULL;
    }
    p->req = req;
    p->seq = seq;
    p->next = NULL;
    if(!reqList)// the first one ,set it as head
    {
        LOG_INFO("insert reqList head success:%d", seq);
        return reqList = p;
    }
    while(ptmp->next != NULL)ptmp = ptmp->next;// move the point to the tail
    ptmp->next = p;
    LOG_INFO("insert reqList success:%d", seq);
    return reqList;
}

/*      delete from reqList where seq is exact, and always return the head*/
REQLIST *remove_reqList(REQLIST *reqList, unsigned char seq)
{
    int i = 0, pos = 0;
    REQLIST *pTemp = NULL,*pLast = NULL,*pNext = NULL;
    if(NULL == reqList)
    {
        LOG_DEBUG("reqList is empty");
        return NULL;
    }
    pTemp = reqList;
    while(pTemp != NULL)
    {
        if(pTemp->seq == seq)
        {
            i = 1;
            break;
        }
        pos++;
        pLast = pTemp;
        pTemp = pTemp->next;
    }
    if(i == 0)
    {
        LOG_DEBUG("reqList has no seq = %d ", seq);
        return reqList;
    }
    if(pos == 0)
    {
        reqList = reqList->next;//remove head
    }
    else
    {
        pNext = pTemp->next;
        pLast->next = pNext;
    }
    free(pTemp);
    LOG_DEBUG("remove reqList success:%d", seq);
    return reqList;
}

/*      select from reqList where seq is exact  */
REQLIST *find_reqList(REQLIST *reqList, unsigned char seq)
{
    int i = 0;
    REQLIST *pTemp = NULL;
    if(NULL == reqList)
    {
        LOG_DEBUG("reqList is empty");
        return NULL;
    }
    pTemp = reqList;
    while(pTemp != NULL)
    {
        if(pTemp->seq == seq)
        {
            i = 1;
            break;
        }
        pTemp = pTemp->next;
    }
    if(i == 0)
    {
        LOG_DEBUG("reqList has no seq = %d ", seq);
        return NULL;
    }
    LOG_DEBUG("find reqList success:%d", seq);
    return pTemp;
}

/* distruct reqList and reply the rest to http*/
REQLIST *distruct_reqList(REQLIST *reqList)
{
    REQLIST *pTmp = NULL, *pLast = NULL;
    pTmp = reqList;
    while(NULL != pTmp)
    {
        pLast = pTmp;
        pTmp = pTmp->next;
        LOG_INFO("distruct reqlist :%d", pTmp->seq);
        http_errorReply(pLast->req, CODE_DEVICE_OFF);
        free(pLast);
    }
    LOG_DEBUG("distruct reqList success");
    return reqList = NULL;
}

