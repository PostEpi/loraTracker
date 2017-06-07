#ifndef __IOTGPS_H__
#define __IOTGPS_H__

#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

bool    makeMessageEvent(char *pdata, int psize, char menufacture, char eventcode, char *pout, int *outsize);
bool    makeMessageGPS(char *pdata, int psize, char *pout, int *outsize );

#ifdef __cplusplus
}
#endif

#endif /* __IOTGPS_H__*/