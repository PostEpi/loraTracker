

#ifndef __MENULIB_H
#define __MENULIB_H

#define OEM_DEBUG_READ_NODATA       -1

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"

uint8_t readByte();
uint8_t WaitKey();
uint32_t GetHexnum32();
uint32_t GetDecnum();
void GetString( uint8_t * string, uint32_t  len	);
int32_t GetIntNum();

#endif  /* __MENULIB_H */