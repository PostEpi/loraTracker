#include <string.h>
#include "rqueue.h"


static RQueue_StatusTypeDef first(rqueue *mm, element item)
{
	int front = ((mm->front - 1) < 0)? mm->q_size - 1: mm->front - 1 % mm->q_size;
	if (front == mm->rear) {
		printf("Queue is Fulll\r\n");
		return RQUEUE_OUTOFMEM;
	}

	__RQ_LOCK(mm);
	mm->count++;
	mm->elements[front] = item;
	//memcpy(&mm->elements[front], item, sizeof(element));
	mm->front = front;
	__RQ_UNLOCK(mm);

	printf("first front=%d, rear=%d, count=%d\r\n", front, mm->rear, mm->count);

	return RQUEUE_OK;
}
static RQueue_StatusTypeDef last(rqueue *mm, element item)
{
	int rear = (mm->rear + 1) % mm->q_size;
	if (mm->front == rear) {
		printf("Queue is Fulll \r\n");
		return RQUEUE_OUTOFMEM;
	}
	__RQ_LOCK(mm);
	mm->count++;
	//memcpy(&mm->elements[mm->rear], item, sizeof(element));
	mm->elements[mm->rear] = item;
	mm->rear = rear;
	__RQ_UNLOCK(mm);

	printf("last front=%d, rear=%d, count=%d\r\n", mm->front, mm->rear, mm->count );

	return RQUEUE_OK;
}
static RQueue_StatusTypeDef removefirst(rqueue *mm, element *item)
{
	if (mm->front == mm->rear) {
		printf("Queue is empty \r\n");
		return RQUEUE_EMPTY;
	}
	else {
		__RQ_LOCK(mm);
		mm->count--;
		//item = &mm->elements[mm->front];
		memcpy(item , &mm->elements[mm->front], sizeof(element));
		mm->front = (mm->front + 1) % mm->q_size;
		__RQ_UNLOCK(mm);

		printf("removefirst front=%d, rear=%d, count=%d \r\n", mm->front, mm->rear, mm->count);
	}

	return RQUEUE_OK;
}

static RQueue_StatusTypeDef get(rqueue *mm, element *item)
{
	int pos = mm->front;
	if (mm->front == mm->rear) {
		printf("Queue is empty \r\n");
		return RQUEUE_EMPTY;
	}
	else {
		memcpy(item , &mm->elements[pos], sizeof(element));
		printf("get front=%d, rear=%d, count=%d \r\n", pos, mm->rear, mm->count);
	}

	return RQUEUE_OK;
}

static void tostring(rqueue *mm)
{

	if (mm->count == 0)
	{
		printf("empty\r\n");
		return;
	}

	element d;
	int front = mm->front;
	while (front != mm->rear)
	{
		d = mm->elements[front];
		printf("%d = [%d %d %s] \r\n", front, d.retcount, d.size, d.edata);
		front = (front + 1) % mm->q_size;
	}
}

rqueue *createRqueue(rqueue* m, element *arrayElement, int size)
{
	//rqueue* m;
	//m = (rqueue*)malloc(sizeof(rqueue));
	m->front = 0;
	m->rear = 0;
	m->count = 0;
	m->q_size = size;
	m->elements = arrayElement;

	// mapping functions.
	m->enqueuefirst = first;
	m->enqueue = last;
	m->dequeue = removefirst;
	m->peek = get;
	m->display = tostring;

	return m;
}