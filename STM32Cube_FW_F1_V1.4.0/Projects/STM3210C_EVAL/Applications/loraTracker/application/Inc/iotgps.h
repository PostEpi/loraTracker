#ifndef __IOTGPS_H__
#define __IOTGPS_H__

#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char    manufacture;
    char    event;
    char    datetime[8];
    float   latitude;
    float   longitude;
    char    direction;
    char    speed;
    short   battordistance;         
} IotGPS_Typedef;


bool    getIotGPSMessage(IotGPS_Typedef *data, char *pout, int *outsize);

#ifdef __cplusplus
}
#endif

#endif /* __IOTGPS_H__*/