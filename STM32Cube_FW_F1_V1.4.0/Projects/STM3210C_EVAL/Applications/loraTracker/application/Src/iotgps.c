#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "iotgps.h"
#include "circledistance.h"
#include "bbox.h"
#include "nmea.h"
#include "debug.h"

void initIotGPS()
{

}

bool getIotGPSMessage(IotGPS_Typedef *data, char *pout, int *outsize)
{
    int formatSize = sizeof(IotGPS_Typedef);
    if(data == NULL || pout == NULL || formatSize > *outsize) 
    {
         DEBUG(ZONE_ERROR, ("getIotGPSMessage : argument invalided value (0x%x 0x%x, %d)@@@@\r\n", data, pout, *outsize));
        return false;
    }

    *outsize = formatSize;
    memset(pout, 0, *outsize);
    memcpy(pout, data, formatSize);

    return true;
}

bool getIotEventMessage(IotEvent_Typedef *data, char *pout, int *outsize)
{
    int formatSize = sizeof(IotEvent_Typedef);
    if(data == NULL || pout == NULL || formatSize > *outsize) 
    {
         DEBUG(ZONE_ERROR, ("getIotEventMessage : argument invalided value (0x%x 0x%x, %d)@@@@\r\n", data, pout, *outsize));
        return false;
    }

    *outsize = formatSize;
    memset(pout, 0, *outsize);
    memcpy(pout, data, formatSize);

    return true;
}

bool getIotUserMessage(IotUser_Typedef *data, char *pout, int *outsize)
{
    int formatSize = sizeof(IotUser_Typedef);
    if(data == NULL || pout == NULL || formatSize > *outsize) 
    {
         DEBUG(ZONE_ERROR, ("getIotUserMessage : argument invalided value (0x%x, 0x%x, %d)@@@@\r\n", data, pout, *outsize));
        return false;
    }

    *outsize = formatSize;
    memset(pout, 0, *outsize);
    memcpy(pout, data, formatSize);

    return true;
}

