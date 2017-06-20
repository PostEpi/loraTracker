#include <stdio.h>
#include <stdlib.h>
#include "iotgps.h"
#include "circledistance.h"
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
    memcpy(pout, data, formatSize);

    return true;
}

bool getIotUserMessage(char *data, int insize, char *pout, int *outsize)
{
    if(data == NULL || pout == NULL || insize > *outsize) 
    {
         DEBUG(ZONE_ERROR, ("getIotUserMessage : argument invalided value (0x%x, %d, 0x%x, %d)@@@@\r\n", data, insize, pout, *outsize));
        return false;
    }

    *outsize = insize;
    *pout = IOTGPS_TYPE_USER;
    strncpy(pout+1, data, insize);

    return true;
}

static void itohex(char *buf, int size, int value)
{
    int i = 0, p = 0;
    char num[10];
    
    sprintf(num, "%x", value);
    
    while(size--) 
    {
        buf[p] = digit2dec(num[i++]) << 4;
        buf[p] += digit2dec(num[i++]);
        p++;
    }
}

static void ftohex(char *buf, int size, float value)
{
    int i = 0, p = 0;
    char num[20];
    
    sprintf(num, "%f", value);
    
    while(size--) 
    {
        buf[p] = digit2dec(num[i++]) << 4;
        buf[p] += digit2dec(num[i++]);
        p++;
    }
}
