/*
 * package_http.c
 *
 *  Created on: 2016/12/02
 *      Author: lc
 */
#include <event.h>
#include <evutil.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "db.h"
#include "log.h"
#include "cJSON.h"
#include "msg_http.h"
#include "package_http.h"

#define MAX_NAME_LEN 32
#define MAX_PATH_LEN 64
#define MAX_LOG_LEN 256
#define READ_BUFFER_LEN 1024

typedef struct
{
    char versionName[MAX_NAME_LEN];
    int versionCode;
    char changeLog[MAX_LOG_LEN];
    char fileName[MAX_NAME_LEN];
    int type;
}APP_PACKAGE_INFO;

static int save_appPackageInfo(const char *versionName, int versionCode, const char *changeLog, const char *fileName, int type, APP_PACKAGE_INFO *pPackageInfo)
{
    strncpy(pPackageInfo->versionName,versionName,MAX_NAME_LEN);
    strncpy(pPackageInfo->changeLog,changeLog,MAX_LOG_LEN);
    strncpy(pPackageInfo->fileName,fileName,MAX_NAME_LEN);
    pPackageInfo->versionCode = versionCode;
    pPackageInfo->type = type;
    return 0;
}

static void package_getVersion(struct evhttp_request *req)
{
    APP_PACKAGE_INFO packageInfo;

    if(db_getAppPackage(save_appPackageInfo, &packageInfo) != 0)
    {
        LOG_ERROR("get app package info error");
        http_errorMsg(req);
        return;
    }

    cJSON *json = cJSON_CreateObject();
    if(!json)
    {
        LOG_FATAL("failed to alloc memory");
        http_errorMsg(req);
        return;
    }

    cJSON_AddStringToObject(json, "versionName", packageInfo.versionName);
    cJSON_AddStringToObject(json, "versionCode", packageInfo.versionCode);
    cJSON_AddStringToObject(json, "changelog", packageInfo.changeLog);

    char *msg = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    LOG_DEBUG("%s",msg);

    http_rspMsg(req, msg);
    free(msg);
    return;
}

static int http_sendPackage(struct evhttp_request * req, const char *filename)
{
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "application/vnd.android.package-archive");
    evhttp_send_reply_start(req, HTTP_OK, "OK");

    char path[MAX_PATH_LEN] = "./app/";

    strcat(path,filename);

    FILE *fp = fopen(path,"r");
    if(!fp)
    {
        LOG_ERROR("open %s error!", path);
        return 1;
    }

    size_t readLen = 0, len = 0;
    char readBuf[READ_BUFFER_LEN + 1] = {0};

    struct evbuffer *buf = evbuffer_new();

    do
    {
        len = fread(readBuf, 1, READ_BUFFER_LEN, fp);
        evbuffer_add(buf, readBuf, len);
        readLen += len;
        fseek(fp, readLen, SEEK_SET);
    }while(len >= READ_BUFFER_LEN);

    fclose(fp);

    evhttp_send_reply_chunk(req,buf);
    evbuffer_free(buf);

    evhttp_add_header(req->output_headers, "Connection", "close");
    evhttp_send_reply_end(req);
    return 0;
}

static void http_getPackage(struct evhttp_request *req)
{
    APP_PACKAGE_INFO packageInfo;

    if(db_getAppPackage(save_appPackageInfo,&packageInfo) != 0)
    {
        LOG_ERROR("get app package info error");
        http_errorMsg(req);
        return;
    }

    if(http_sendPackage(req, packageInfo.fileName) != 0)
    {
        http_errorMsg(req);
    }
    return;
}


void http_replyVersion(struct evhttp_request *req)
{
    LOG_INFO("%s",req->uri);
    switch(req->type)
    {
        case EVHTTP_REQ_GET:
            {
                package_getVersion(req);
                return;
            }
            break;

        case EVHTTP_REQ_PUT:
        case EVHTTP_REQ_POST:
        case EVHTTP_REQ_DELETE:
        default:
            LOG_ERROR("unkown http type: %d",req->type);
            break;
    }

    http_errorMsg(req);
    return;
}


void http_replyPackage(struct evhttp_request *req)
{
    LOG_INFO("%s",req->uri);
    switch(req->type)
    {
        case EVHTTP_REQ_GET:
            {
                 http_getPackage(req);
                 return;
            }
            break;

        case EVHTTP_REQ_PUT:
        case EVHTTP_REQ_POST:
        case EVHTTP_REQ_DELETE:
        default:
            LOG_ERROR("unkown http type: %d",req->type);
            break;
    }

    http_errorMsg(req);
    return;
}


