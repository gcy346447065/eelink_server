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

static char LastFileName[256];
static int LastFileSize = 0;

int getLastVersionWithFileNameAndSizeStored(void)
{
    DIR *dir_handle;
    struct dirent *ptr;
    int a = 0, b = 0, c = 0, NowVersion = 0, LastVersion = 0;

    dir_handle = opendir("./firmware/");
    if(!dir_handle)
    {
        return -1;
    }

    while(ptr = readdir(dir_handle))
    {
        if(ptr->d_type == 8 && sscanf(ptr->d_name, "app_%d.%d.%d", &a, &b, &c) == 3)//d_type == 8 means file
        {
            //NowVersion = (a << 16 | b << 8 | c);
            NowVersion = a *100 + b *10 + c;

            if(NowVersion > LastVersion)
            {
                LOG_INFO("a is %d, b is %d, c is %d", a, b, c);

                LastVersion = NowVersion;

                memset(LastFileName, 0, 256);
                sprintf(LastFileName, "./firmware/%s", ptr->d_name);
            }
        }
    }

    if(LastVersion != 0)
    {
        LOG_INFO("LastFileName is %s, LastVersion is %d", LastFileName, LastVersion);

        struct stat buf;
        if(stat(LastFileName, &buf) < 0)
        {
            LOG_ERROR("stat errno is %d", errno);
            return 0;
        }

        LastFileSize = (int)buf.st_size;

        LOG_INFO("LastFileSize is %d", LastFileSize);
    }

    closedir(dir_handle);
    return LastVersion;
}

int getLastFileSize(void)
{
    if(LastFileSize > 0)
    {
        return LastFileSize;
    }
    else
    {
        return 0;
    }
}

int getDataSegmentWithGottenSize(int gottenSize, char *data, int *pSendSize)
{
    if(gottenSize < LastFileSize)
    {
        int delta = LastFileSize - gottenSize;
        if(delta >= FIRMWARE_SEGMENT_SIZE)
        {
            //send FIRMWARE_SEGMENT_SIZE
            int fd = open(LastFileName, O_RDONLY);
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
            int fd = open(LastFileName, O_RDONLY);
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
    else if(gottenSize == LastFileSize)
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

int getLastFileChecksum(void)
{
    char *data = malloc(LastFileSize);
    int fd = open(LastFileName, O_RDONLY);
    int readSize = read(fd, data, 0);

    int checksum = adler32((unsigned char *)data, LastFileSize);

    LOG_INFO("readSize is %d, LastFileSize is %d, checksum is %d", readSize, LastFileSize, checksum);

    return checksum;
}
