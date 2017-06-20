#ifndef __IOTGPS_H__
#define __IOTGPS_H__

#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOTGPS_TYPE_GPSDATA     1
#define IOTGPS_TYPE_EVENT       2
#define IOTGPS_TYPE_USER        3

typedef struct {
    char    type;
    char    manufacture;
    char    event;
    int     datetime;
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