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
         DEBUG(ZONE_ERROR, ("getIotGPSMessage : argument invalided value (0x%x 0x%x, %d)@@@@\r\n", data, pout, outsize));
        return false;
    }

    *outsize = formatSize;
    memcpy(pout, data, formatSize);

    return true;
}