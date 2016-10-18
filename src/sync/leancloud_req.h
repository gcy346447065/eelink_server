/*
 * leancloud_req.h
 *
 *  Created on: Apr 25, 2015
 *      Author: jk
 */

#ifndef SRC_LEANCLOUD_REQ_H_
#define SRC_LEANCLOUD_REQ_H_

int leancloud_saveDid(const char* imei);

int leancloud_saveGPS(int timestamp, const char* imei, double lat, double lng, int speed, int course);

int leancloud_saveGPSWithDID(int timestamp, const char *imei, double lat, double lng, int speed, int course, const char *did);

int leancloud_saveItinerary(const char *imei, int miles);

int leancloud_saveSimInfo(const char* imei, const char* ccid, const char* imsi);

int leancloud_getOBJ(void);

int leancloud_getObjectIDWithImei(const char* imei);

#endif /* SRC_LEANCLOUD_REQ_H_ */
