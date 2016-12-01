/*
 * mc_object.h
 *
 *  Created on: Apr 19, 2015
 *      Author: jk
 */

#ifndef SRC_OBJECT_
#define SRC_OBJECT_

#include "macro.h"
#include "session.h"


typedef struct
{
//    char DID[MAX_DID_LEN];
//    char clientID[CLIENT_ID_LEN];
    short cmd;
    unsigned short seq;
}APP_SESSION;


typedef struct
{
    /*
     * IMEI = TAC(6) + FAC(2) + SNR(6) + SP(1)
     * TAC: Type Approval Code �ͺź�׼����
     * FAC: Final Assembly Code ���װ���
     * SNR: Serial Number
     * SP: У����
     *
     * We use the SNR for MAC
     */

    int ObjectType;
    int version;

    char IMEI[IMEI_LENGTH + 1];
    char CCID[CCID_LENGTH + 1];
    char IMSI[IMSI_LENGTH + 1];
    char language;
    char locale;

    int timestamp;
    float lat;
    float lon;
    char speed;
    short course;

    CGI_MC cell[CELL_NUM];
    char isGPSlocated;

    char gsm;
    char voltage;
    short analog1;
    short analog2;

    char DID[MAX_DID_LEN];
    char pwd[MAX_PWD_LEN];

    char m2m_host[100];
    int m2m_Port;

    int device_id;
    int sensor_id;

    //gps开关，为刘老师写字版本增加
    int gps_switch;

    void *session;

    //itinerary
    char isStarted;
    char timecount;
    int starttime;
    float startlat;
    float startlon;
    int itineray;
} OBJECT;

typedef int (*SIMCOM_SAVEITINERARY_PROC)(const char* tableName, int starttime, float startlat, float startlon, int endtime, float endlat, float endlon, short itinerary);
typedef int (*MANAGER_SEND_PROC)(const void *msg, const void *sessionManager, const char*imei, const char on_offline, int version, int timestamp, float lat, float lon, char speed, short course);

void obj_sendImeiData2ManagerLoop(const void *msg, const void *sessionManager, MANAGER_SEND_PROC func);

void obj_table_initial(void (*mqtt_sub)(const char *), int ObjectType);
void obj_table_GPSinitial(void);
void obj_table_ItieraryJudge(void *arg);
void obj_table_destruct();

OBJECT *obj_new();
void obj_add(OBJECT *obj);
void obj_del(OBJECT *obj);
OBJECT *obj_get(const char IMEI[]);

int obj_did_got(OBJECT *obj);
const char* get_IMEI_STRING(const char* IMEI);
const char* getMacFromIMEI(const unsigned char* IMEI);



#endif /* SRC_OBJECT_ */
