#ifndef __BBOX_H__
#define __BBOX_H__

#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BBOX_MESSAGE_SIZE   40
#define BBOX_STX_BERDP      "$BERDP"
#define BBOX_STR_BURDP      "$BURDP" 

typedef enum {
    BBOX_NONE_MESSAGE,
    BBOX_ENVET_MESSAGE,
    BBOX_USER_MESSAGE   
}BBOX_MessageTypeDef;

bool    parsebbox(char *pdata, int psize);

// READER functions: retrieving results, call isbboxready() first
int     digit2dec(char digit);
float   string2float(char* s);
bool	isbboxready();
char	getbboxManufacture();
char	getbboxEvent();

#ifdef __cplusplus
}
#endif

#endif /* __BBOX_H__*/