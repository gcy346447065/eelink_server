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

/*      initail reqList         */
REQLIST *init_reqList(REQLIST *reqList)
{
    return reqList = NULL;
}

/*   insert into reqList what req and seq needed, it doesnt matter when reqList is null  */
int insert_reqList(REQLIST *reqList, struct evhttp_request *req, unsigned char seq)
{
    REQLIST *ptmp = reqList;
    REQLIST *p = (REQLIST *)malloc(sizeof(REQLIST));
    if(!p)
    {
        LOG_ERROR("create req failed");
        return 1;
    }
    p->req = req;
    p->seq = seq;
    p->next = NULL;

    if(!reqList)// the first one ,set it as head
    {
        reqList = p;
        return 0;
    }

    while(ptmp->next != NULL)ptmp = ptmp->next;// move the point to the tail

    ptmp->next = p;

    LOG_INFO("insert reqList success");
    return 0;
}

/*      delete from reqList where seq is exact  */
void remove_reqList(REQLIST *reqList, unsigned char seq)
{
    int i = 0, pos = 0;
    REQLIST *pTemp,*pLast,*pNext;

    if(NULL == reqList)
    {
        LOG_DEBUG("reqList is empty");
        return;
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
        return;
    }

    if(pos == 0)
    {
        reqList = reqList->next;//remove head ,here pLast is NULL
    }
    else
    {
        pNext = pTemp->next;
        pLast->next = pNext;
    }
    LOG_INFO("remove reqList success");

    free(pTemp);
    return;
}

/*      delete from reqList where seq is exact  */
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

    LOG_INFO("find reqList success");

    return pTemp;
}

int size_reqList(REQLIST *reqList)
{
    int length = 0;
    REQLIST *ptmp = reqList;
    while(NULL != ptmp)
    {
        length++;
        ptmp = ptmp->next;
    }

    return length;
}

/* distruct reqList and reply the rest to http*/
int distruct_reqList(REQLIST *reqList)
{
    REQLIST *pTmp, *pLast;
    pTmp = reqList;
    while(NULL != pTmp)
    {
        pLast = pTmp;
        pTmp = pTmp->next;
        http_errorReply(pLast->req, CODE_DEVICE_OFF);
        free(pLast);
    }

    reqList = NULL;

    LOG_INFO("distruct reqList success");
    return 0;
}

