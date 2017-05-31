#include "demd.h"
#include "vdb.h"
#include "timtimer.h"
#include "timeServer.h"
#include "gcommand.h"

static TimerEvent_t reportToserverTimer;

static void OnReportToServerTimer(void);

void DEMD_Init()
{
    TimerInit(&reportToserverTimer, OnReportToServerTimer);
	TimerSetValue(&reportToserverTimer, 60000); /* 1000ms */
	TimerStart(&reportToserverTimer);
}
void DEMD_Process()
{
    char *cmd;
    element item;

    if (!isEmptydDB(DEM) && selectDB(DEM, &item) == RQUEUE_OK)
    {
        DEBUG(ZONE_TRACE, ("DEMD_Process : %d, %d, %s\r\n", item.size, item.retcount, item.edata ));
        DEBUG(ZONE_TRACE, ("%s", item.edata));
        deleteDB(DEM, &item);

        // parser 

        if(updateDB(LOR, item.edata, item.size, true) != RQUEUE_OK)
        {
            DEBUG(ZONE_ERROR, ("DEMD_Process : Update is failed to LOR @@@@ \r\n"));
        }
    }
}

DEMD_StatusTypeDef DEMD_IOcontrol(DEMD_IOControlTypedef io, int *input, int insize, int *output, int *outsize)
{
    DEMD_StatusTypeDef ret = DEMD_OK;

    DEBUG(ZONE_FUNCTION, ("+DEMD_IOcontrol %d, 0x%x, %d, 0x%x, %d\r\n", io, input, insize, output, outsize));

    switch (io)
    {
    case DEMD_REPORT_PERIOD_CHANGE:
        // parser for input.
        if(input != NULL && size > 0)
        {
            TimerStop(&reportToserverTimer);
            TimerSetValue(&reportToserverTimer, 120000);
            TimerStart(&reportToserverTimer);
        }
        break;
    case DEMD_REPORT_TO_SERVER_FAILED:
        // 
        
        break;
    default:
        ret = DEMD_PARAM_ERROR;
    }

    DEBUG(ZONE_FUNCTION, ("-DEMD_IOcontrol %d\r\n", ret));
    return ret;
}

static void OnReportToServerTimer(void)
{
   	GCMD_IOcontrol(GCOM_REPORT_REQUEST, NULL, 0, NULL, 0);
	TimerStart(&reportToserverTimer);
}