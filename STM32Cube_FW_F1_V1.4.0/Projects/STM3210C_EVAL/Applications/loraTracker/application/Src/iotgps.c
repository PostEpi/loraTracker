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

void displayTranferMessage(const char *data, int size)
{
    int value;
    DEBUG(ZONE_FUNCTION, ("transfer : "));
    for( int i = 0; i < size ; i++)
    {
         value = ((*data & 0xf0) >> 4);
         if( value < 0xa) {
           DEBUG(ZONE_FUNCTION, ("%c", value + 48));
         } else {
           DEBUG(ZONE_FUNCTION, ("%c", value + 55));
         }

                  value = (*data & 0xf);

         if( value < 0xa) {
           DEBUG(ZONE_FUNCTION, ("%c", value + 48));
         } else {
           DEBUG(ZONE_FUNCTION, ("%c", value + 55));
         }
         data++;
    }
    DEBUG(ZONE_FUNCTION, ("\r\n"));

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

    displayTranferMessage(pout, formatSize);

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

    displayTranferMessage(pout, formatSize);

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

    displayTranferMessage(pout, formatSize);
    return true;
}

