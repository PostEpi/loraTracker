#ifndef __IOTGPS_H__
#define __IOTGPS_H__

#include "stm32f1xx.h"
#include "bbox.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IOT_TYPE_NONE = 0,
    IOT_TYPE_GPSDATA = 1,
    IOT_TYPE_EVENT = 2, 
    IOT_TYPE_USER = 3,
    IOT_TYPE_DUMMY = 16
} IOT_MessageTypeDef;


typedef __packed struct {
    char    type;
    char    version;
    char    reportcycle;
    int     datetime;
    int     latitude;
    int     longitude;
    char    direction;
    char    speed;
    short   cumulativedistance;         
    char    statusofcar;         
} IotGPS_Typedef;

typedef __packed struct {
    char    type;
    char    serviceversion;
    char    manufacture;
    char    event;
    double  eventinfo;
    int     battery;         
    char    temperature;
    char    boxserial[MAX_SIZE_OF_BBOX_COLUME];
    int     datetime;
    int     latitude;
    int     longitude;
    char    direction;
    char    speed;
} IotEvent_Typedef;

typedef __packed struct {
    char    type;
    char    manufacture;
    char    event[22];
} IotUser_Typedef;

#define IOT_GPS_SPECIFICATION           2

bool    getIotGPSMessage(IotGPS_Typedef *data, char *pout, int *outsize);
bool    getIotEventMessage(IotEvent_Typedef *data, char *pout, int *outsize);
bool    getIotUserMessage(IotUser_Typedef *data, char *pout, int *outsize);

#ifdef __cplusplus
}
#endif

#endif /* __IOTGPS_H__*/