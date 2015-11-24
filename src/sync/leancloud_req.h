/*
 * leancloud_req.h
 *
 *  Created on: Apr 25, 2015
 *      Author: jk
 */

#ifndef SRC_LEANCLOUD_REQ_H_
#define SRC_LEANCLOUD_REQ_H_

void leancloud_saveDid(const char* imei);

void leancloud_saveGPS(const char* imei, double lat, double lng);

int leancloud_getOBJ();

#endif /* SRC_LEANCLOUD_REQ_H_ */
