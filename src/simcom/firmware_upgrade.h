/*
 * firmware_upgrade.h
 *
 *  Created on: Feb 28, 2016
 *      Author: gcy
 */

#ifndef FIRMWARE_UPGRADE_H_
#define FIRMWARE_UPGRADE_H_

int getDataSliceWithGottenSize(int version, int gottenSize, char *data, int *pSendSize);
int getFirmwarePkg(int version, char *LastFileNamewithPath);
int getLastFileChecksum(int version);
int getFirmwarePkgVersion(int version);
int getFirmwarePkgSize(int version);

#endif /* FIRMWARE_UPGRADE_H_ */
