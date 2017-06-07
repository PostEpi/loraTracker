#ifndef __BBOX_H__
#define __BBOX_H__

#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BBOX_MESSAGE_SIZE   10
#define BBOX_STX_STRING     "$BERDP"

bool    parsebbox(char *pdata, int psize);

// READER functions: retrieving results, call isbboxready() first
bool	isbboxready();
char	getbboxManufacture();
char	getbboxEvent();

#ifdef __cplusplus
}
#endif

#endif /* __BBOX_H__*/