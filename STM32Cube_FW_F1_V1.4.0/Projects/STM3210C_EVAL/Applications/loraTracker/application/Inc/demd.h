#ifndef __DEMD_H__
#define __DEMD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "wisol.h"

#define DEMD_IOT_MESSAGE_SIZE       WISOL_USER_DATA_LIMIT
//#define DEMD_IOT_SK_TEST_SPEC
//#define DEMD_IOT_SK_PREPARE_SPEC
//#define DEMO_IOT_DONTCARE_GPSDATA
//#define DEMO_IOT_RF_TX_MODE_FOR_MASSPRODUCT
//#define DEMO_IOT_RF_RX_MODE_FOR_MASSPRODUCT

typedef enum {
    DEMD_OK = 0,
    DEMD_ERROR,
    DEMD_BUSSY,
    DEMD_PARAM_ERROR,
} DEMD_StatusTypeDef;

typedef enum {
    DEMD_REPORT_PERIOD_CHANGE = 0,
    DEMD_REPORT_TO_SERVER_FAILED,
    DEMD_REPORT_TO_SERVER_SUCCEED,
    DEMD_REPORT_NETWORK_READY,
    DEMD_REPORT_PREV_GPS,
    DEMD_REPORT_PREV_LATITUDE,
    DEMD_REPORT_PREV_LONGITUDE
} DEMD_IOControlTypedef;

typedef struct {
    float latitude;
    float longitude;
} DEMD_GPSTypeDef;

void DEMD_Init();
void DEMD_Process();
DEMD_StatusTypeDef DEMD_IOcontrol(DEMD_IOControlTypedef io, int *input, int insize, int *output, int *outsize); 
void evaluateGPS(char *pdata, int psize);


#ifdef __cplusplus
}
#endif

#endif /* __DEMD_H__*/
