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

int leancloud_saveItinerary(const char *imei, int start, int end, int miles);

int leancloud_getOBJ(void);

int leancloud_sendSms2Tel(const char *SmsTemplate, const char *TelNumber);

#endif /* SRC_LEANCLOUD_REQ_H_ */
