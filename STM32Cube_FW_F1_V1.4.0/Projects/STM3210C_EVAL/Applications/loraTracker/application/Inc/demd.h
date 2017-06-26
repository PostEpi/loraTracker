#ifndef __DEMD_H__
#define __DEMD_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DEMD_IOT_MESSAGE_SIZE       50

typedef enum {
    DEMD_OK = 0,
    DEMD_ERROR,
    DEMD_BUSSY,
    DEMD_PARAM_ERROR,
} DEMD_StatusTypeDef;

typedef enum {
    DEMD_REPORT_PERIOD_CHANGE = 0,
    DEMD_REPORT_TO_SERVER_FAILED,
    DEMD_REPORT_TO_SERVER_SUCCEED
} DEMD_IOControlTypedef;


void DEMD_Init();
void DEMD_Process();
DEMD_StatusTypeDef DEMD_IOcontrol(DEMD_IOControlTypedef io, int *input, int insize, int *output, int *outsize); 

#ifdef __cplusplus
}
#endif

#endif /* __DEMD_H__*/
