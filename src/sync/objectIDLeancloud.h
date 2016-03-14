/*
 * objectIDLeancloud.h
 *
 *  Created on: Mar 11, 2016
 *      Author: gcy
 */
#ifndef OBJECTID_LEANCLOUD_H_
#define OBJECTID_LEANCLOUD_H_

void objectID_table_initial(void);
void objectID_table_destruct(void);
void objectID_add_hash(const char *imei, const char *objectID);
void objectID_del_hash(const char *imei);


#endif /* OBJECTID_LEANCLOUD_H_ */
