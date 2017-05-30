#ifndef __VDB_H__
#define __VDB_H__

#include "stm32f1xx.h"
#include "rqueue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
  LOR = 0,
  GPS,
  EXT,
  DB_MAX_SIZE
} DB_TypeDef;

RQueue_StatusTypeDef updateDB(DB_TypeDef db, char *data, int size, bool mustack);
RQueue_StatusTypeDef deleteDB(DB_TypeDef db, element *item);
RQueue_StatusTypeDef selectDB(DB_TypeDef db, element *item);
bool isEmptydDB(DB_TypeDef db);

#ifdef __cplusplus
}
#endif

#endif /* __VDB_H__*/