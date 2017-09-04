#ifndef __BBOX_H__
#define __BBOX_H__

#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BBOX_MESSAGE_SIZE           80
#define MAX_SIZE_OF_BBOX_ROW        9
#define MAX_SIZE_OF_BBOX_COLUME     16
#define BBOX_STX_BDRDP              "$BDRDP"
#define BBOX_STX_BERDP              "$BERDP"
#define BBOX_STR_BURDP              "$BURDP" 

bool    parsebbox(const char *pdata, int psize);

// READER functions: retrieving results, call isbboxready() first
bool	isbboxready();
int     getUserMessage(char *buf, int size);
char	getbboxManufacture();
char	getbboxEvent();

#ifdef __cplusplus
}
#endif

#endif /* __BBOX_H__*/