/*
 * yeelink_req.h
 *
 *  Created on: Apr 30, 2015
 *      Author: jk
 */

#ifndef SRC_YEELINK_REQ_H_
#define SRC_YEELINK_REQ_H_

#include "object_mc.h"

void yeelink_createDevice(OBJ_MC* obj, void* arg);
void yeelink_createSensor(int device_id, void* arg);

void yeelink_saveGPS(OBJ_MC* obj, void* arg);

#endif /* SRC_YEELINK_REQ_H_ */
