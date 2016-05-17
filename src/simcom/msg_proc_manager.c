/*
 * msg_proc_manager.c
 *
 *  Created on: 2016/4/15
 *      Author: gcy
 */
#include <string.h>

#include "log.h"
#include "macro.h"
#include "object.h"
//#include "session.h"
#include "protocol.h"
#include "msg_simcom.h"
#include "msg_manager.h"
#include "session_manager.h"
#include "protocol_manager.h"
#include "msg_proc_manager.h"

typedef int (*MANAGER_MSG_PROC)(const void *msg, SESSION_MANAGER *ctx);
typedef struct
{
    char cmd;
    MANAGER_MSG_PROC pfn;
} MANAGER_MSG_PROC_MAP;

static int manager_sendMsg(void *msg, size_t len, SESSION_MANAGER *sessionManager)
{
    if (!sessionManager)
    {
        return -1;
    }

    MSG_SEND pfn = sessionManager->pSendMsg;
    if (!pfn)
    {
        LOG_ERROR("device offline");
        return -1;
    }
    pfn(sessionManager->bev, msg, len);

    LOG_DEBUG("send msg(cmd=%d), length(%ld)", get_manager_msg_cmd(msg), len);
    LOG_HEX(msg, len);
    free_manager_msg(msg);

    return 0;
}

int manager_sendImeiData(const void *msg, SESSION_MANAGER *sessionManager, const char *imei, SESSION *deviceSession, int timestamp, float lon, float lat, char speed, short course)
{
    char online_offline;
    if(deviceSession == NULL)
    {
        online_offline = 2; //offline
    }
    else
    {
        online_offline = 1; //online
    }

    LOG_INFO("manager_sendImeiData, imei(%s)", imei);

    MANAGER_MSG_IMEI_DATA_RSP *rsp = (MANAGER_MSG_IMEI_DATA_RSP *)alloc_manager_rspMsg((const MANAGER_MSG_HEADER *)msg);
    if(rsp)
    {
        memcpy(rsp->imei_data.IMEI, imei, MANAGER_MAX_IMEI_LENGTH);
        rsp->imei_data.online_offline = online_offline;
        rsp->imei_data.gps.timestamp = timestamp;
        rsp->imei_data.gps.longitude = lon;
        rsp->imei_data.gps.latitude = lat;
        rsp->imei_data.gps.speed = speed;
        rsp->imei_data.gps.course = course;

        manager_sendMsg(rsp, sizeof(MANAGER_MSG_IMEI_DATA_RSP), sessionManager);
        LOG_INFO("send imei data rsp");
    }
    else
    {
        LOG_ERROR("insufficient memory");
        free_manager_msg(rsp);
        return -1;
    }

    return 0;
}

static int manager_login(const void *msg, SESSION_MANAGER *sessionManager)
{
    const MANAGER_MSG_LOGIN_REQ *req = (const MANAGER_MSG_LOGIN_REQ *)msg;
    if(ntohs(req->length) != sizeof(MANAGER_MSG_LOGIN_REQ) - MANAGER_MSG_HEADER_LEN)
    {
        LOG_ERROR("login message length not enough");
        return -1;
    }

    //login rsp
    MANAGER_MSG_LOGIN_RSP *rsp = (MANAGER_MSG_LOGIN_RSP *)alloc_manager_rspMsg((const MANAGER_MSG_HEADER *)msg);
    if(rsp)
    {
        manager_sendMsg(rsp, sizeof(MANAGER_MSG_LOGIN_RSP), sessionManager);
        LOG_INFO("send login rsp");
    }
    else
    {
        free_manager_msg(rsp);
        LOG_ERROR("insufficient memory");
        return -1;
    }

    return 0;
}

static int manager_imeiData(const void *msg, SESSION_MANAGER *sessionManager)
{
    const MANAGER_MSG_IMEI_DATA_REQ *req = (const MANAGER_MSG_IMEI_DATA_REQ *)msg;
    if(ntohs(req->header.length) < sizeof(MANAGER_MSG_IMEI_DATA_REQ) - MANAGER_MSG_HEADER_LEN)
    {
        LOG_ERROR("imei data message length not enough");
        return -1;
    }

    char imei[IMEI_LENGTH + 1];
    memcpy(imei, req->IMEI, IMEI_LENGTH);
    imei[IMEI_LENGTH] = '\0'; //add '\0' for string operaton

    LOG_INFO("get imei data req, imei(%s)", imei);

    //imei data rsp
    MANAGER_MSG_IMEI_DATA_RSP *rsp = (MANAGER_MSG_IMEI_DATA_RSP *)alloc_manager_rspMsg((const MANAGER_MSG_HEADER *)msg);
    if(rsp)
    {
        OBJECT *obj = obj_get(imei);
        if(obj)
        {
            LOG_INFO("succeed to find imei(%s) in object, send full rsp", imei);

            memcpy(rsp->imei_data.IMEI, imei, MANAGER_MAX_IMEI_LENGTH);
            rsp->imei_data.online_offline = obj->session ? 1 : 2;//1 for online, 2 for offline
            rsp->imei_data.version = obj->version;
            rsp->imei_data.gps.timestamp = obj->timestamp;
            rsp->imei_data.gps.longitude = obj->lon;
            rsp->imei_data.gps.latitude = obj->lat;
            rsp->imei_data.gps.speed = obj->speed;
            rsp->imei_data.gps.course = obj->course;
        }
        else
        {
            LOG_INFO("failed to find imei(%s) in object, send null rsp", imei);
        }
        
        manager_sendMsg(rsp, sizeof(MANAGER_MSG_IMEI_DATA_RSP), sessionManager);
        LOG_INFO("send imei data rsp");
    }
    else
    {
        LOG_ERROR("insufficient memory");
        return -1;
    }

    return 0;
}

static int manager_getLog(const void *msg, SESSION_MANAGER *sessionManager)
{
    const MANAGER_MSG_GET_LOG_REQ *req = (const MANAGER_MSG_GET_LOG_REQ *)msg;
    if(ntohs(req->header.length) != sizeof(MANAGER_MSG_GET_LOG_REQ) - MANAGER_MSG_HEADER_LEN)
    {
        LOG_ERROR("get log message length not enough");
        return -1;
    }

    char imei[IMEI_LENGTH + 1];
    memcpy(imei, req->IMEI, IMEI_LENGTH);
    imei[IMEI_LENGTH] = '\0'; //add '\0' for string operaton

    LOG_INFO("get log req, imei(%s)", imei);

    //send req msg to simcom
    OBJECT *obj = obj_get(imei);
    if(obj)
    {
        LOG_INFO("succeed to find imei(%s) in object, send req to simcom", imei);

        MSG_GET_LOG_REQ *req4simcom = (MSG_GET_LOG_REQ *)alloc_simcomManagerReq(CMD_GET_LOG);
        if(!req4simcom)
        {
            LOG_FATAL("insufficient memory");
            return -1;
        }
        req4simcom->managerSeq = sessionManager->sequence;

        SESSION *simcomSession = (SESSION *)obj->session;
        if (!simcomSession)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        MSG_SEND pfn = simcomSession->pSendMsg;
        if (!pfn)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        LOG_HEX(req4simcom, sizeof(MSG_GET_LOG_REQ));
        pfn(simcomSession->bev, req4simcom, sizeof(MSG_GET_LOG_REQ)); //simcom_sendMsg
    }
    else
    {
        LOG_INFO("failed to find imei(%s) in object, don't send req", imei);
        return -1;
    }

    return 0;
}

static int manager_get433(const void *msg, SESSION_MANAGER *sessionManager)
{
    const MANAGER_MSG_GET_433_REQ *req = (const MANAGER_MSG_GET_433_REQ *)msg;
    if(ntohs(req->header.length) != sizeof(MANAGER_MSG_GET_433_REQ) - MANAGER_MSG_HEADER_LEN)
    {
        LOG_ERROR("get 433 message length not enough");
        return -1;
    }

    char imei[IMEI_LENGTH + 1];
    memcpy(imei, req->IMEI, IMEI_LENGTH);
    imei[IMEI_LENGTH] = '\0'; //add '\0' for string operaton

    LOG_INFO("get 433 req, imei(%s)", imei);

    //send req msg to simcom
    OBJECT *obj = obj_get(imei);
    if(obj)
    {
        LOG_INFO("succeed to find imei(%s) in object, send req to simcom", imei);

        MSG_GET_433_REQ *req4simcom = (MSG_GET_433_REQ *)alloc_simcomManagerReq(CMD_GET_433);
        if(!req4simcom)
        {
            LOG_FATAL("insufficient memory");
            return -1;
        }
        req4simcom->managerSeq = sessionManager->sequence;

        SESSION *simcomSession = (SESSION *)obj->session;
        if (!simcomSession)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        MSG_SEND pfn = simcomSession->pSendMsg;
        if (!pfn)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        LOG_HEX(req4simcom, sizeof(MSG_GET_433_REQ));
        pfn(simcomSession->bev, req4simcom, sizeof(MSG_GET_433_REQ)); //simcom_sendMsg
    }
    else
    {
        LOG_INFO("failed to find imei(%s) in object, don't send req", imei);
        return -1;
    }

    return 0;
}

static int manager_getGSM(const void *msg, SESSION_MANAGER *sessionManager)
{
    const MANAGER_MSG_GET_GSM_REQ *req = (const MANAGER_MSG_GET_GSM_REQ *)msg;
    if(ntohs(req->header.length) != sizeof(MANAGER_MSG_GET_GSM_REQ) - MANAGER_MSG_HEADER_LEN)
    {
        LOG_ERROR("get GSM message length not enough");
        return -1;
    }

    char imei[IMEI_LENGTH + 1];
    memcpy(imei, req->IMEI, IMEI_LENGTH);
    imei[IMEI_LENGTH] = '\0'; //add '\0' for string operaton

    LOG_INFO("get GSM req, imei(%s)", imei);

    //send req msg to simcom
    OBJECT *obj = obj_get(imei);
    if(obj)
    {
        LOG_INFO("succeed to find imei(%s) in object, send req to simcom", imei);

        MSG_GET_GSM_REQ *req4simcom = (MSG_GET_GSM_REQ *)alloc_simcomManagerReq(CMD_GET_GSM);
        if(!req4simcom)
        {
            LOG_FATAL("insufficient memory");
            return -1;
        }
        req4simcom->managerSeq = sessionManager->sequence;

        SESSION *simcomSession = (SESSION *)obj->session;
        if (!simcomSession)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        MSG_SEND pfn = simcomSession->pSendMsg;
        if (!pfn)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        LOG_HEX(req4simcom, sizeof(MSG_GET_GSM_REQ));
        pfn(simcomSession->bev, req4simcom, sizeof(MSG_GET_GSM_REQ)); //simcom_sendMsg
    }
    else
    {
        LOG_INFO("failed to find imei(%s) in object, don't send req", imei);
        return -1;
    }

    return 0;
}

static int manager_getGPS(const void *msg, SESSION_MANAGER *sessionManager)
{
    const MANAGER_MSG_GET_GPS_REQ *req = (const MANAGER_MSG_GET_GPS_REQ *)msg;
    if(ntohs(req->header.length) != sizeof(MANAGER_MSG_GET_GPS_REQ) - MANAGER_MSG_HEADER_LEN)
    {
        LOG_ERROR("get GPS message length not enough");
        return -1;
    }

    char imei[IMEI_LENGTH + 1];
    memcpy(imei, req->IMEI, IMEI_LENGTH);
    imei[IMEI_LENGTH] = '\0'; //add '\0' for string operaton

    LOG_INFO("get GPS req, imei(%s)", imei);

    //send req msg to simcom
    OBJECT *obj = obj_get(imei);
    if(obj)
    {
        LOG_INFO("succeed to find imei(%s) in object, send req to simcom", imei);

        MSG_GET_GPS_REQ *req4simcom = (MSG_GET_GPS_REQ *)alloc_simcomManagerReq(CMD_GET_GPS);
        if(!req4simcom)
        {
            LOG_FATAL("insufficient memory");
            return -1;
        }
        req4simcom->managerSeq = sessionManager->sequence;

        SESSION *simcomSession = (SESSION *)obj->session;
        if (!simcomSession)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        MSG_SEND pfn = simcomSession->pSendMsg;
        if (!pfn)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        LOG_HEX(req4simcom, sizeof(MSG_GET_GPS_REQ));
        pfn(simcomSession->bev, req4simcom, sizeof(MSG_GET_GPS_REQ)); //simcom_sendMsg
    }
    else
    {
        LOG_INFO("failed to find imei(%s) in object, don't send req", imei);
        return -1;
    }

    return 0;
}

static int manager_getSetting(const void *msg, SESSION_MANAGER *sessionManager)
{
    const MANAGER_MSG_GET_SETTING_REQ *req = (const MANAGER_MSG_GET_SETTING_REQ *)msg;
    if(ntohs(req->header.length) != sizeof(MANAGER_MSG_GET_SETTING_REQ) - MANAGER_MSG_HEADER_LEN)
    {
        LOG_ERROR("get setting message length not enough");
        return -1;
    }

    char imei[IMEI_LENGTH + 1];
    memcpy(imei, req->IMEI, IMEI_LENGTH);
    imei[IMEI_LENGTH] = '\0'; //add '\0' for string operaton

    LOG_INFO("get setting req, imei(%s)", imei);

    //send req msg to simcom
    OBJECT *obj = obj_get(imei);
    if(obj)
    {
        LOG_INFO("succeed to find imei(%s) in object, send req to simcom", imei);

        MSG_GET_SETTING_REQ *req4simcom = (MSG_GET_SETTING_REQ *)alloc_simcomManagerReq(CMD_GET_SETTING);
        if(!req4simcom)
        {
            LOG_FATAL("insufficient memory");
            return -1;
        }
        req4simcom->managerSeq = sessionManager->sequence;

        SESSION *simcomSession = (SESSION *)obj->session;
        if (!simcomSession)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        MSG_SEND pfn = simcomSession->pSendMsg;
        if (!pfn)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        LOG_HEX(req4simcom, sizeof(MSG_GET_SETTING_REQ));
        pfn(simcomSession->bev, req4simcom, sizeof(MSG_GET_SETTING_REQ)); //simcom_sendMsg
    }
    else
    {
        LOG_INFO("failed to find imei(%s) in object, don't send req", imei);
        return -1;
    }

    return 0;
}

static int manager_getBattery(const void *msg, SESSION_MANAGER *sessionManager)
{
    const MANAGER_MSG_GET_BATTERY_REQ *req = (const MANAGER_MSG_GET_BATTERY_REQ *)msg;
    if(ntohs(req->header.length) != sizeof(MANAGER_MSG_GET_BATTERY_REQ) - MANAGER_MSG_HEADER_LEN)
    {
        LOG_ERROR("get battery message length not enough");
        return -1;
    }

    char imei[IMEI_LENGTH + 1];
    memcpy(imei, req->IMEI, IMEI_LENGTH);
    imei[IMEI_LENGTH] = '\0'; //add '\0' for string operaton

    LOG_INFO("get battery req, imei(%s)", imei);

    //send req msg to simcom
    OBJECT *obj = obj_get(imei);
    if(obj)
    {
        LOG_INFO("succeed to find imei(%s) in object, send req to simcom", imei);

        MSG_GET_BATTERY_REQ *req4simcom = (MSG_GET_BATTERY_REQ *)alloc_simcomManagerReq(CMD_GET_BATTERY);
        if(!req4simcom)
        {
            LOG_FATAL("insufficient memory");
            return -1;
        }
        req4simcom->managerSeq = sessionManager->sequence;

        SESSION *simcomSession = (SESSION *)obj->session;
        if (!simcomSession)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        MSG_SEND pfn = simcomSession->pSendMsg;
        if (!pfn)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        LOG_HEX(req4simcom, sizeof(MSG_GET_BATTERY_REQ));
        pfn(simcomSession->bev, req4simcom, sizeof(MSG_GET_BATTERY_REQ)); //simcom_sendMsg
    }
    else
    {
        LOG_INFO("failed to find imei(%s) in object, don't send req", imei);
        return -1;
    }

    return 0;
}

static int manager_reboot(const void *msg, SESSION_MANAGER *sessionManager)
{
    const MANAGER_MSG_REBOOT_REQ *req = (const MANAGER_MSG_REBOOT_REQ *)msg;
    if(ntohs(req->header.length) != sizeof(MANAGER_MSG_REBOOT_REQ) - MANAGER_MSG_HEADER_LEN)
    {
        LOG_ERROR("get reboot message length not enough");
        return -1;
    }

    char imei[IMEI_LENGTH + 1];
    memcpy(imei, req->IMEI, IMEI_LENGTH);
    imei[IMEI_LENGTH] = '\0'; //add '\0' for string operaton

    LOG_INFO("get reboot req, imei(%s)", imei);

    //send req msg to simcom
    OBJECT *obj = obj_get(imei);
    if(obj)
    {
        LOG_INFO("succeed to find imei(%s) in object, send req to simcom", imei);

        MSG_REBOOT_REQ *req4simcom = (MSG_REBOOT_REQ *)alloc_simcomManagerReq(CMD_REBOOT);
        if(!req4simcom)
        {
            LOG_FATAL("insufficient memory");
            return -1;
        }

        SESSION *simcomSession = (SESSION *)obj->session;
        if (!simcomSession)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        MSG_SEND pfn = simcomSession->pSendMsg;
        if (!pfn)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        LOG_HEX(req4simcom, sizeof(MSG_REBOOT_REQ));
        pfn(simcomSession->bev, req4simcom, sizeof(MSG_REBOOT_REQ)); //simcom_sendMsg
    }
    else
    {
        LOG_INFO("failed to find imei(%s) in object, don't send req", imei);
        return -1;
    }

    return 0;
}

static int manager_upgrade(const void *msg, SESSION_MANAGER *sessionManager)
{
    const MANAGER_MSG_UPGRADE_REQ *req = (const MANAGER_MSG_UPGRADE_REQ *)msg;
    if(ntohs(req->header.length) != sizeof(MANAGER_MSG_UPGRADE_REQ) - MANAGER_MSG_HEADER_LEN)
    {
        LOG_ERROR("get upgrade message length not enough");
        return -1;
    }

    char imei[IMEI_LENGTH + 1];
    memcpy(imei, req->IMEI, IMEI_LENGTH);
    imei[IMEI_LENGTH] = '\0'; //add '\0' for string operaton

    LOG_INFO("get upgrade req, imei(%s)", imei);

    //send req msg to simcom
    OBJECT *obj = obj_get(imei);
    if(obj)
    {
        LOG_INFO("succeed to find imei(%s) in object, send req to simcom", imei);

        MSG_UPGRADE_REQ *req4simcom = (MSG_UPGRADE_REQ *)alloc_simcomManagerReq(CMD_UPGRADE);
        if(!req4simcom)
        {
            LOG_FATAL("insufficient memory");
            return -1;
        }

        SESSION *simcomSession = (SESSION *)obj->session;
        if (!simcomSession)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        MSG_SEND pfn = simcomSession->pSendMsg;
        if (!pfn)
        {
            LOG_ERROR("device offline");
            return -1;
        }
        LOG_HEX(req4simcom, sizeof(MSG_UPGRADE_REQ));
        pfn(simcomSession->bev, req4simcom, sizeof(MSG_UPGRADE_REQ)); //simcom_sendMsg
    }
    else
    {
        LOG_INFO("failed to find imei(%s) in object, don't send req", imei);
        return -1;
    }

    return 0;
}

static MANAGER_MSG_PROC_MAP msgProcs[] =
{
    {MANAGER_CMD_LOGIN,             manager_login},
    {MANAGER_CMD_IMEI_DATA,         manager_imeiData},
    {MANAGER_CMD_GET_LOG,           manager_getLog},
    {MANAGER_CMD_GET_433,           manager_get433},
    {MANAGER_CMD_GET_GSM,           manager_getGSM},
    {MANAGER_CMD_GET_GPS,           manager_getGPS},
    {MANAGER_CMD_GET_SETTING,       manager_getSetting},
    {MANAGER_CMD_GET_BATTERY,       manager_getBattery},
    {MANAGER_CMD_REBOOT,            manager_reboot},
    {MANAGER_CMD_UPGRADE,           manager_upgrade}
};

static int handle_one_msg(const void *m, SESSION_MANAGER *ctx)
{
    const MANAGER_MSG_HEADER *msg = (const MANAGER_MSG_HEADER *)m;

    for (size_t i = 0; i < sizeof(msgProcs) / sizeof(msgProcs[0]); i++)
    {
        if (msgProcs[i].cmd == msg->cmd)
        {
            MANAGER_MSG_PROC pfn = msgProcs[i].pfn;
            if (pfn)
            {
                return pfn(msg, ctx);
            }
        }
    }

    return -1;
}

int handle_manager_msg(const char *m, size_t msgLen, void *arg)
{
    const MANAGER_MSG_HEADER *msg = (const MANAGER_MSG_HEADER *)m;

    if(msgLen < MANAGER_MSG_HEADER_LEN)
    {
        LOG_ERROR("message length not enough: %zu(at least(%zu))", msgLen, MANAGER_MSG_HEADER_LEN);

        return -1;
    }
    size_t leftLen = msgLen;
    while(leftLen >= ntohs(msg->length) + MANAGER_MSG_HEADER_LEN)
    {
        const unsigned char *status = (const unsigned char *)(&(msg->signature));
        if((status[0] != 0xaa) || (status[1] != 0x66))
        {
            LOG_ERROR("receive message header signature error:%x", (unsigned)ntohs(msg->signature));
            return -1;
        }
        handle_one_msg(msg, (SESSION_MANAGER *)arg);
        leftLen = leftLen - MANAGER_MSG_HEADER_LEN - ntohs(msg->length);
        msg = (const MANAGER_MSG_HEADER *)(m + msgLen - leftLen);
    }
    return 0;
}
