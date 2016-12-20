/*
 * macro_mc.h
 *
 *  Created on: Apr 19, 2015
 *      Author: jk
 */

#ifndef SRC_MACRO_H_
#define SRC_MACRO_H_

#define IMEI_LENGTH (15)
#define CCID_LENGTH (20)
#define IMSI_LENGTH (15)
#define OBJECT_ID_LENGTH (24)
#define MAX_DID_LEN (24)
#define MAX_PWD_LEN (16)
#define TELNUMBER_LENGTH (12)

#define MAC_MAC_LEN 6

#define PRODUCT_KEY "01fdd12699454be1a072094ec063749d"
#define MAGIC_NUMBER 0x12345678
#define CELL_NUM 7

#define FIRMWARE_SEGMENT_SIZE 1024

typedef struct
{
	short mcc;	//mobile country code
	short mnc;	//mobile network code
	short lac;	//local area code
	short ci; //cell id,modified. why the length is 3, not 2 ???
	short rxl;	//receive level
}__attribute__((__packed__)) CGI_MC;

enum
{
    ObjectType_null = 0,
    ObjectType_tk115 = 1,
    ObjectType_simcom = 2
};

#endif /* SRC_MACRO_H_ */
