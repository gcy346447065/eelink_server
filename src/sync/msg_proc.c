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
        LOG_ERROR("save Did failed");
        return;
    }

    leancloud_saveDid(imei->valuestring);

    return;
}

static void msg_saveGPS(cJSON* json)
{
    cJSON* timestamp = cJSON_GetObjectItem(json, TAG_TIMESTAMP);
    cJSON* imei = cJSON_GetObjectItem(json, TAG_IMEI);
    cJSON* lat = cJSON_GetObjectItem(json, TAG_LAT);
    cJSON* lng = cJSON_GetObjectItem(json, TAG_LNG);
    cJSON* speed = cJSON_GetObjectItem(json, TAG_SPEED);
    cJSON* course = cJSON_GetObjectItem(json, TAG_COURSE);

    if (!timestamp || !imei || !lat || !lng || !speed || !course)
    {
        LOG_ERROR("save GPS failed");
        return;
    }

    leancloud_saveGPS(timestamp->valueint,
                      imei->valuestring,
                      lat->valuedouble,
                      lng->valuedouble,
                      speed->valueint,
                      course->valueint);

    return;
}

static void msg_saveItinerary(cJSON* json)
{
    cJSON* imei = cJSON_GetObjectItem(json, TAG_IMEI);
    cJSON* start = cJSON_GetObjectItem(json, TAG_START);
    cJSON* end = cJSON_GetObjectItem(json, TAG_END);
    cJSON* miles = cJSON_GetObjectItem(json, TAG_MILES);
    if (!imei || !start || !end || !miles)
    {
        LOG_ERROR("save Itinerary failed");
        return;
    }

    leancloud_saveItinerary(imei->valuestring,
                            start->valueint,
                            end->valueint,
                            miles->valueint);

    return;
}

static void msg_saveSimInfo(cJSON* json)
{
    cJSON* imei = cJSON_GetObjectItem(json, TAG_IMEI);
    cJSON* ccid = cJSON_GetObjectItem(json, TAG_CCID);
    cJSON* imsi = cJSON_GetObjectItem(json, TAG_IMSI);

    if (!imei || !ccid || !imsi)
    {
        LOG_ERROR("save SimInfo failed");
        return;
    }

    leancloud_saveSimInfo(imei->valuestring,
                          ccid->valuestring,
                          imsi->valuestring);

    return;
}

#if 0
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

        case CMD_SYNC_NEW_ITINERARY:
            msg_saveItinerary(root);
            break;

        case CMD_SYNC_NEW_SIM_INFO:
            msg_saveSimInfo(root);
            break;

        default:
            break;
    }

    cJSON_Delete(root);

    return 0;
}
#endif

int handle_incoming_msg(const char *m, size_t msgLen, void *arg)
{
    const char *msg = m;
    cJSON *root;

    while(root = cJSON_ParseWithOpts(msg, &msg, 0))
    {
        cJSON* cmd = cJSON_GetObjectItem(root, TAG_CMD);

        switch (cmd->valueint)
        {
            case CMD_SYNC_NEW_DID:
                msg_saveDid(root);
                break;

            case CMD_SYNC_NEW_GPS:
                msg_saveGPS(root);
                break;

            case CMD_SYNC_NEW_ITINERARY:
                msg_saveItinerary(root);
                break;

            case CMD_SYNC_NEW_SIM_INFO:
                msg_saveSimInfo(root);
                break;

            default:
                break;
        }

        cJSON_Delete(root);
    }

    return 0;
}
