/*
 * msg_gizwits.h
 *
 *  Created on: 2015��4��23��
 *      Author: jk
 */

#ifndef SRC_MSG_GIZWITS_H_
#define SRC_MSG_GIZWITS_H_

typedef struct
{
	char sub_cmd;

	char readOnlyData;
	char speed;
	char mnc;
	short course;
	short mcc;
	short lac;
	short ci;
	int lat;
	int lon;

	char checksum; //TODO: is this field needed???
}__attribute__((__packed__)) GIZWITS_DATA;



#endif /* SRC_MSG_GIZWITS_H_ */
