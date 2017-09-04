
#ifndef __CARSTATUS_H__
#define __CARSTATUS_H__

#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CAR_STATUS_NONE = 0,
    CAR_STATUS_DRIVING,
    CAR_STATUS_PARKING,
    CAR_STATUS_STARTING,
    CAR_STATUS_STOPING,
} CarStatus_Typedef;

#define TICKTIMERDEVIATION          10000
#define TIMEMIN                     60000      // 
#define TIMEOUTSPEED                10*TIMEMIN+TICKTIMERDEVIATION
#define TIMEOUTOUTOFSERVICE         20*TIMEMIN+2*TICKTIMERDEVIATION

#ifdef __cplusplus
}
#endif

#endif /* __CARSTATUS_H__ */