//
// Created by jk on 15-10-30.
//


#include "msg_proc.h"
#include "cJSON.h"
#include "log.h"
#include "inter_msg.h"
#include "leancloud_req.h"

static void msg_saveDid(cJSON* json)
{
    cJSON* imei = cJSON_GetObjectItem(json, TAG_IMEI);

    if (!imei)
    {
        //TODO: log error
        return;
    }

    leancloud_saveDid(imei->valuestring);

    return;
}

static void msg_saveGPS(cJSON* json)
{
    cJSON* imei = cJSON_GetObjectItem(json, TAG_IMEI);

    cJSON* lat = cJSON_GetObjectItem(json, TAG_LAT);
    cJSON* lng = cJSON_GetObjectItem(json, TAG_LNG);

    if (!imei || !lat || !lng)
    {
        //TODO: log error
        return;
    }

    leancloud_saveGPS(imei->valuestring, lat->valuedouble, lng->valuedouble);

    return;
}

int handle_incoming_msg(const char *m, size_t msgLen, void *arg)
{

    cJSON* root = cJSON_Parse(m);
    if (!root)
    {
        LOG_ERROR("error parse respone");
        return -1;
    }

    cJSON* cmd = cJSON_GetObjectItem(root, TAG_CMD);

    switch (cmd->valueint)
    {
        case CMD_SYNC_NEW_DID:
            msg_saveDid(root);
            break;

        case CMD_SYNC_NEW_GPS:
            msg_saveGPS(root);
            break;

        default:
            break;
    }

    return 0;
}
