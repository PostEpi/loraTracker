/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RQUEUE_H
#define __RQUEUE_H

#include "stm32f1xx_hal_def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RQUEUE_OK = 0x00,
	RQUEUE_ERROR = 0x01,
	RQUEUE_BUSY = 0x02,
	RQUEUE_EMPTY = 0x03,
	RQUEUE_OUTOFMEM = 0x04

} RQueue_StatusTypeDef;

typedef enum {
	UNLOCKED = 0x00,
	LOCKED = 0x01
} RQueue_RLockTypeDef;

#define __RQ_LOCK(__HANDLE__)             \
	do                                    \
	{                                     \
		if ((__HANDLE__)->Lock == LOCKED) \
		{                                 \
			return RQUEUE_BUSY;           \
		}                                 \
		else                              \
		{                                 \
			(__HANDLE__)->Lock = LOCKED;  \
		}                                 \
	} while (0)

#define __RQ_UNLOCK(__HANDLE__)        \
	do                                 \
	{                                  \
		(__HANDLE__)->Lock = UNLOCKED; \
	} while (0)

typedef struct
{
	int retcount;
	int size;
	char edata[100];
} element;

typedef struct rqueue
{
	int dbname;

	int front;
	int rear;
	int count;
	int q_size;
	RQueue_RLockTypeDef Lock; /*!< Locking object */
	element *elements;

	RQueue_StatusTypeDef (*enqueuefirst)(struct rqueue *mm, element item);
	RQueue_StatusTypeDef (*enqueue)(struct rqueue *mm, element item);
	RQueue_StatusTypeDef (*dequeue)(struct rqueue *mm, element *item);
	RQueue_StatusTypeDef (*peek)(struct rqueue *mm, element *item);
	void (*display)(struct rqueue *mm);
} rqueue;

rqueue *createRqueue(rqueue* m, int dbname, element *arrayElement, int size);
// RQueue_StatusTypeDef first(rqueue *mm, element item);
// RQueue_StatusTypeDef last(rqueue *mm, element item);
// RQueue_StatusTypeDef removefirst(rqueue *mm, element *item);
// RQueue_StatusTypeDef get(rqueue *mm, element *item);
// void tostring(rqueue *mm);

#ifdef __cplusplus
}
#endif

#endif /* __RQUEUE_H */
