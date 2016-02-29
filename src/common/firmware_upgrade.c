/*
 * firmware_upgrade.c
 *
 *  Created on: Feb 28, 2016
 *      Author: gcy
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "firmware_upgrade.h"
#include "macro.h"
#include "log.h"

static char *LastFileName = NULL;

int getLastVersionAndSize(int *LastVersion, int *size)
{
    DIR *dir_handle;
    struct dirent *ptr;
    int a = 0, b = 0, c = 0, NowVersion = 0, TempVersion = 0;

    //enter the dir
    dir_handle = opendir("./firmware/");
    if(!dir_handle)
    {
        return -1;
    }

    //loop for the lastest version
    while(ptr = readdir(dir_handle))
    {
        //LOG_INFO("d_name is %s, d_type is %d", ptr->d_name, ptr->d_type);

        if(ptr->d_type == 8 && sscanf(ptr->d_name, "app_%d.%d.%d", &a, &b, &c) == 3)//d_type == 8 means file
        {
            NowVersion = (a << 16 | b << 8 | c);

            if(NowVersion > TempVersion)
            {
                LOG_INFO("a is %d, b is %d, c is %d", a, b, c);
                TempVersion = NowVersion;
                LastFileName = ptr->d_name;
            }
        }
    }

    if(TempVersion != 0)
    {
        *LastVersion = TempVersion;

        LOG_INFO("LastFileName is %s, LastVersion is %d", LastFileName, *LastVersion);

        struct stat buf;
        if(stat(LastFileName, &buf) < 0)
        {
            LOG_ERROR("stat errno is %d", errno);
            return -1;
        }

        *size = (int)buf.st_size;

        LOG_INFO("last file size is %d", *size);
    }

    closedir(dir_handle);
    return 0;
}

int getDataSegmentFromLastFile()
{

    FIRMWARE_SEGMENT_SIZE;

    return 0;
}


