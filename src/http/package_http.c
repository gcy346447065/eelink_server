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

static int save_appPackageInfo(const char *versionName, int versionCode, const char *changeLog, const char *fileName, int type, void *userdata)
{
    APP_PACKAGE_INFO *app_packageInfo = userdata;

    strncpy(app_packageInfo->versionName,versionName,MAX_NAME_LEN);
    strncpy(app_packageInfo->changeLog,changeLog,MAX_LOG_LEN);
    strncpy(app_packageInfo->fileName,fileName,MAX_NAME_LEN);
    app_packageInfo->versionCode = versionCode;
    app_packageInfo->type = type;
    return 0;
}

static void package_getVersion(struct evhttp_request *req)
{
    APP_PACKAGE_INFO *app_packageInfo = (APP_PACKAGE_INFO *)malloc(sizeof(APP_PACKAGE_INFO));
    if(!app_packageInfo)
    {
        LOG_ERROR("failed to alloc memory");
        http_errorMsg(req);
        return;
    }

    if(db_getAppPackage(save_appPackageInfo,app_packageInfo) != 0)
    {
        LOG_ERROR("get app package info error");
        http_errorMsg(req);
        free(app_packageInfo);
        return;
    }

    cJSON *json = cJSON_CreateObject();
    if(!json)
    {
        LOG_FATAL("failed to alloc memory");
        http_errorMsg(req);
        free(app_packageInfo);
        return;
    }

    cJSON_AddStringToObject(json, "versionName", app_packageInfo->versionName);
    cJSON_AddStringToObject(json, "versionCode", app_packageInfo->versionCode);
    cJSON_AddStringToObject(json, "changelog", app_packageInfo->changeLog);

    char *msg = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    LOG_DEBUG("%s",msg);

    http_rspMsg(req, msg);
    free(app_packageInfo);
    free(msg);
    return;
}


static void http_getPackage(struct evhttp_request *req)
{
    APP_PACKAGE_INFO *app_packageInfo = (APP_PACKAGE_INFO *)malloc(sizeof(APP_PACKAGE_INFO));
    if(!app_packageInfo)
    {
        LOG_ERROR("failed to alloc memory");
        http_errorMsg(req);
        return;
    }

    if(db_getAppPackage(save_appPackageInfo,app_packageInfo) != 0)
    {
        LOG_ERROR("get app package info error");
        http_errorMsg(req);
        free(app_packageInfo);
        return;
    }

    evhttp_add_header(req->output_headers, "Server", "http v1");
    evhttp_add_header(req->output_headers, "Content-Type", "application/vnd.android.package-archive");
    evhttp_send_reply_start(req, HTTP_OK, "OK");

    char path[MAX_PATH_LEN] = "./app/";

    strcat(path,app_packageInfo->fileName);

    FILE *fp = fopen(path,"r");
    if(!fp)
    {
        LOG_ERROR("open %s error!", path);
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

    evhttp_send_reply_end(req);
    free(app_packageInfo);
    return;
}


void http_replyVersion(struct evhttp_request *req)
{
    int rc;
    char telNumber[TELNUMBER_LENGTH + 1] = {0};

    LOG_INFO("%s",req->uri);
    switch(req->type)
    {
        case EVHTTP_REQ_GET:
        case EVHTTP_REQ_PUT:
        case EVHTTP_REQ_POST:
        case EVHTTP_REQ_DELETE:
            {
                package_getVersion(req);
                return;
            }
            break;

        default:
            LOG_ERROR("unkown http type: %d",req->type);
            break;
    }

    http_errorMsg(req);
    return;
}


void http_replyPackage(struct evhttp_request *req)
{
    int rc;
    char telNumber[TELNUMBER_LENGTH + 1] = {0};

    LOG_INFO("%s",req->uri);
    switch(req->type)
    {
        case EVHTTP_REQ_GET:
        case EVHTTP_REQ_PUT:
        case EVHTTP_REQ_POST:
        case EVHTTP_REQ_DELETE:
            {
                 http_getPackage(req);
                 return;
            }
            break;

        default:
            LOG_ERROR("unkown http type: %d",req->type);
            break;
    }

    http_errorMsg(req);
    return;
}


