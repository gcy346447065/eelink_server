/*
 * msg_app.h
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */

#ifndef SRC_MSG_APP_H_
#define SRC_MSG_APP_H_


//Message definition
typedef struct
{
	short header;
	short cmd;
	unsigned short length;
	unsigned short seq;
	char data[];
}__attribute__((__packed__)) APP_MSG;


typedef struct
{
	short header;
	int timestamp;
	int lat;
	int lng;
	short course;
	char speed;
	char isGPS;
}__attribute__((__packed__)) GPS_MSG;


#endif /* SRC_MSG_APP_H_ */
