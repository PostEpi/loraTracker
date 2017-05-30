#ifndef __NMEA_H__
#define __NMEA_H__

#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif


int		fusedata(char c);

// READER functions: retrieving results, call isdataready() first
bool	isdataready();
int		getHour();
int		getMinute();
int		getSecond();
int		getDay();
int		getMonth();
int		getYear();
float	getLatitude();
float	getLongitude();
int		getSatellites();
float	getAltitude();
float	getSpeed();
float	getBearing()

#ifdef __cplusplus
}
#endif

#endif /* __NMEA_H__*/