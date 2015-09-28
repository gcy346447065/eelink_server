/*
 * macro_mc.h
 *
 *  Created on: Apr 19, 2015
 *      Author: jk
 */

#ifndef SRC_MACRO_MC_H_
#define SRC_MACRO_MC_H_

#define IMEI_LENGTH (8)
#define MAX_DID_LEN (24)
#define DID_LEN (22)
#define CLIENT_ID_LEN (32)
#define MAX_PWD_LEN (16)

#define MAC_MAC_LEN 6

#define PRODUCT_KEY "01fdd12699454be1a072094ec063749d"
#define MAGIC_NUMBER 0x12345678

typedef struct
{
	short mcc;	//mobile country code
	short mnc;	//mobile network code
	short lac;	//local area code
	char ci[3]; //cell id, why the length is 3, not 2 ???
}__attribute__((__packed__)) CGI;

#endif /* SRC_MACRO_MC_H_ */
