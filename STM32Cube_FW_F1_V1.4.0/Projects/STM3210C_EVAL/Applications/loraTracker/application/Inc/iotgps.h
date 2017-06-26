#ifndef __IOTGPS_H__
#define __IOTGPS_H__

#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IOT_TYPE_NONE = 0,
    IOT_TYPE_GPSDATA = 1,
    IOT_TYPE_EVENT = 2, 
    IOT_TYPE_USER = 3
} IOT_MessageTypeDef;


typedef struct {
    char    type;
    char    version;
    char    reportcycle;
    int     datetime;
    float   latitude;
    float   longitude;
    char    direction;
    char    speed;
    short   cumulativedistance;         
    char    statusofcar;         
} IotGPS_Typedef;

typedef struct {
    char    type;
    char    manufacture;
    char    event;
    int     datetime;
    float   latitude;
    float   longitude;
    char    direction;
    char    speed;
    char    battery;         
} IotEvent_Typedef;

typedef struct {
    char    type;
    char    manufacture;
    char    event[22];
} IotUser_Typedef;

bool    getIotGPSMessage(IotGPS_Typedef *data, char *pout, int *outsize);
bool    getIotEventMessage(IotEvent_Typedef *data, char *pout, int *outsize);
bool    getIotUserMessage(IotUser_Typedef *data, char *pout, int *outsize);

#ifdef __cplusplus
}
#endif

#endif /* __IOTGPS_H__*/