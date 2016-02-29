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
#include "firmware_upgrade.h"
#include "macro.h"
#include "log.h"

int getLastVersionAndSize(int *LastVersion, int *size)
{
    DIR *dir_handle;
    struct dirent *ptr;
    int a = 0, b = 0, c = 0, NowVersion = 0, TempVersion = 0;
    char *TempName = NULL;

    //enter the dir
    dir_handle = opendir("./firmware/");
    if(!dir_handle)
    {
        return -1;
    }

    //loop for the lastest version
    while(ptr = readdir(dir_handle))
    {
        LOG_INFO("name is %s", ptr->d_name);

        if(sscanf(ptr->d_name, "app_%d.%d.%d", a, b, c) == 3)
        {
            NowVersion = (a << 16 | b << 8 | c);

            if(NowVersion > TempVersion)
            {
                TempVersion = NowVersion;
                TempName = ptr->d_name;
            }
        }
    }

    if(TempVersion != 0)
    {
        *LastVersion = TempVersion;

        struct stat buf;
        stat(TempName, &buf);
        *size = (int)buf.st_size;

        LOG_INFO("LastVersion is %d, size is %d", *LastVersion, *size);
    }

    closedir(dir_handle);
    return 0;
}

/*
OpenAppFileWithLastVersion()
{
    return;
}
*/

