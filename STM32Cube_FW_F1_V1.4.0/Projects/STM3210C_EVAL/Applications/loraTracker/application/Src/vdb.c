#include "string.h"
#include "vdb.h"
#include "debug.h"

#define NODE_GPS_BUFFER 10

static rqueue g_db[DB_MAX_SIZE];
static element arrayElements[DB_MAX_SIZE][NODE_GPS_BUFFER];

static bool initialDB = false;

static void init() 
{
    int i;
    DEBUG(ZONE_FUNCTION, ("init\r\n"));
    for(i = 0; i < DB_MAX_SIZE; i++)
    {
        createRqueue(&g_db[i], i, &arrayElements[i][0], NODE_GPS_BUFFER);
    }
    initialDB = true;
}

RQueue_StatusTypeDef updateDB(DB_TypeDef db, char *data, int size, int needack)
{
    RQueue_StatusTypeDef status = RQUEUE_ERROR;

    if(initialDB == false) 
    {
        init();
    }
    if(db >= DB_MAX_SIZE) 
    {
        DEBUG(ZONE_ERROR, ("Error : There is no database name\r\n"))
        return status;
    }

    rqueue *rq = &g_db[db];

    if(data != NULL && size)
    {
        int limit = 100;
        element item;
        memcpy(item.edata ,data, size);
        item.size = size;
        item.retcount = needack; 

        do 
        {
            status = rq->enqueue(rq, item);
            //rq->display(rq);
        } while(status == RQUEUE_BUSY && limit--);
        
        if(limit < 1) 
        {
            DEBUG(ZONE_ERROR, ("Error : updateDB is failed !!! "))
        }
    }
    
    return status;
}

RQueue_StatusTypeDef deleteDB(DB_TypeDef db, element *item)
{
    RQueue_StatusTypeDef status = RQUEUE_ERROR;
    if(initialDB == false) 
    {
        init();
    }
    if(db >= DB_MAX_SIZE) 
    {
        DEBUG(ZONE_ERROR, ("Error : There is no database name\r\n"))
        return status;
    }

    rqueue *rq = &g_db[db];

    if(item != NULL) 
    {
        int limit = 100;
        do 
        {
            status = rq->dequeue(rq, item);
            //rq->display(rq);
        } while(status == RQUEUE_BUSY && limit--);
        if(limit < 1) 
        {
            DEBUG(ZONE_ERROR, ("Error : deleteDB is failed !!! "))

        }
    }
    
    return status;
}

RQueue_StatusTypeDef selectDB(DB_TypeDef db, element *item)
{
    RQueue_StatusTypeDef status = RQUEUE_ERROR;

    if(initialDB == false) 
    {
        init();
    }
    if(db >= DB_MAX_SIZE) 
    {
        DEBUG(ZONE_ERROR, ("Error : There is no database name\r\n"))
        return status;
    }

    rqueue *rq = &g_db[db];

    if(item != NULL) 
    {
        status = rq->peek(rq, item);
        //rq->display(rq);
    }

    return status;
}

bool isEmptydDB(DB_TypeDef db)
{
    if(initialDB == false) 
    {
        init();
    }
    if(db >= DB_MAX_SIZE) 
    {
        DEBUG(ZONE_ERROR, ("Error : There is not dabaase name\r\n"));
        return true;
    }

    rqueue *rq = &g_db[db];
    //DEBUG(ZONE_TRACE, ("dabaase count = %d\r\n", rq->count));
    (rq->count == 0)? true : false;
}