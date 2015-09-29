/*
 * msg_app.h
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */

#ifndef SRC_MSG_APP_H_
#define SRC_MSG_APP_H_

enum CMD
{
	CMD_WILD		= 0x00,
	CMD_FENCE_SET	= 0x01,
	CMD_FENCE_DEL	= 0x02,
	CMD_FENCE_GET	= 0x03,
	CMD_SEEK_ON		= 0x04,
	CMD_SEEK_OFF	= 0x05,
	CMD_GPS_GET		= 0x06,
};

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
	float lat;
	float lon;
	short course;
	char speed;
	char isGPS;
}__attribute__((__packed__)) GPS_MSG;

typedef struct
{
	short header;
	int intensity;
}__attribute__((__packed__)) F33_MSG;//433

#endif /* SRC_MSG_APP_H_ */
