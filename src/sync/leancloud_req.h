/*
 * leancloud_req.h
 *
 *  Created on: Apr 25, 2015
 *      Author: jk
 */

#ifndef SRC_LEANCLOUD_REQ_H_
#define SRC_LEANCLOUD_REQ_H_

int leancloud_saveDid(const char* imei);

int leancloud_saveGPS(const char* imei, double lat, double lng);

int leancloud_getOBJ(void);

int leancloud_sendSms2Tel(char *SmsTemplate, char *TelNumber);

#endif /* SRC_LEANCLOUD_REQ_H_ */
