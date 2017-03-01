/*
 * firmware_upgrade.c
 *
 *  Created on: Feb 28, 2016
 *      Author: gcy
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "firmware_upgrade.h"
#include "adler32.h"
#include "macro.h"
#include "log.h"

/*
*function getFirmwarePkgVersion: get the last app version
*param:version input
*return:Lastversion
*/
int getFirmwarePkgVersion(int version)
{
    int Lastversion = 0;
    char firmwarePkg[128] = {0};// no use, just Intermediate variable

    if(0 != db_getFirmwarePkg(version, &Lastversion, firmwarePkg))
    {
        LOG_DEBUG("No Firmware Package in Database");
        return 0;
    }

    return Lastversion;
}


/*
*function getFirmwarePkgSize: get the last app filesize
*param:version input, LastFileNamewithPath output no use as NULL
*return: filesize
*/
int getFirmwarePkg(int version, char *LastFileNamewithPath)
{
    struct stat buf;
    int LastVersion = 0;
    char firmwarePkg[128] = {0};

    if(0 != db_getFirmwarePkg(version, &LastVersion, firmwarePkg))
    {
        LOG_INFO("No Firmware Package in Database");
        return -1;
    }

    if(LastFileNamewithPath)
    {
        snprintf(LastFileNamewithPath, 256, "./firmware/%s", firmwarePkg);
        if(stat(LastFileNamewithPath, &buf) < 0)
        {
            LOG_ERROR("stat errno is %d", errno);
            return -1;
        }
    }
    else
    {
        char FileNamewithPath[256] = {0};
        snprintf(FileNamewithPath, 256, "./firmware/%s", firmwarePkg);
        if(stat(FileNamewithPath, &buf) < 0)
        {
            LOG_ERROR("stat errno is %d", errno);
            return -1;
        }
    }

    return buf.st_size;
}

int getDataSliceWithGottenSize(int version, int gottenSize, char *data, int *pSendSize)
{
    char LastFileNamewithPath[256] = {0};
    int filesize = getFirmwarePkg(version, LastFileNamewithPath);
    if(filesize < 0)
    {
        return -1;
    }

    if(gottenSize < filesize)
    {
        int delta = filesize - gottenSize;
        if(delta >= FIRMWARE_SEGMENT_SIZE)
        {
            //send FIRMWARE_SEGMENT_SIZE
            int fd = open(LastFileNamewithPath, O_RDONLY);
            if(!fd)
            {
                LOG_ERROR("open file errno(%d)", errno);
                return -1;
            }

            lseek(fd, gottenSize, SEEK_SET);
            int sendSize = read(fd, data, FIRMWARE_SEGMENT_SIZE);
            close(fd);

            LOG_INFO("sendSize = %d", sendSize);

            if(sendSize < 0 || (sendSize > 0 && sendSize > FIRMWARE_SEGMENT_SIZE))
            {
                LOG_ERROR("read file errno(%d)", errno);
                return -1;
            }
            else if(sendSize == 0 || (sendSize > 0 && sendSize <= FIRMWARE_SEGMENT_SIZE))
            {
                LOG_INFO("read file ok");
                *pSendSize = sendSize;
            }
        }
        else
        {
            //send delta
            int fd = open(LastFileNamewithPath, O_RDONLY);
            if(!fd)
            {
                LOG_ERROR("open file errno(%d)", errno);
                return -1;
            }

            lseek(fd, gottenSize, SEEK_SET);
            int sendSize = read(fd, data, delta);
            close(fd);

            LOG_INFO("sendSize = %d", sendSize);

            if(sendSize == delta)
            {
                LOG_INFO("read file ok");
                *pSendSize = sendSize;
            }
            else
            {
                LOG_ERROR("read file errno(%d)", errno);
                return -1;
            }
        }
    }
    else if(gottenSize == filesize)
    {
        LOG_INFO("gottenSize == LastFileSize, upgrade ok");
    }
    else
    {
        LOG_ERROR("gottenSize > LastFileSize");
        return -1;
    }

    return 0;
}

int getLastFileChecksum(int version)
{
    char LastFileNamewithPath[256] = {0};
    int LastFileSize = getFirmwarePkg(version, LastFileNamewithPath);

    unsigned char *data = malloc(LastFileSize);
    int fd = open(LastFileNamewithPath, O_RDONLY);
    int readSize = read(fd, data, LastFileSize);

    int checksum = adler32(data, readSize);

    LOG_INFO("readSize is %d, LastFileSize is %d, checksum is %d", readSize, LastFileSize, checksum);

    return checksum;
}


