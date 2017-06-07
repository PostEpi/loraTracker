#include "demd.h"
#include "vdb.h"
#include "timtimer.h"
#include "timeServer.h"
#include "gcommand.h"
#include "bbox.h"



static TimerEvent_t reportToserverTimer;
static bool reportBBox = false;

static void OnReportToServerTimer(void);
static bool getIotGPSMessage(char *pdata, int psize, char *pout, int *poutsize);

void DEMD_Init()
{
    // get previous message sent to sever from flash.
    
    TimerInit(&reportToserverTimer, OnReportToServerTimer);
	TimerSetValue(&reportToserverTimer, 60000); /* 1000ms */
	TimerStart(&reportToserverTimer);


}

void DEMD_Process()
{
    char *cmd;
    element item;
    static char iotMessage[DEMD_IOT_MESSAGE_SIZE];
    static int iotSize = DEMD_IOT_MESSAGE_SIZE;

    if (!isEmptydDB(DEM) && selectDB(DEM, &item) == RQUEUE_OK)
    {
        DEBUG(ZONE_TRACE, ("DEMD_Process : %d, %d, %s\r\n", item.size, item.retcount, item.edata ));
        deleteDB(DEM, &item);

        if(getIotGPSMessage(item.edata, item.size, iotMessage, &iotSize))
        {
            if(updateDB(LOR, iotMessage, iotSize, true) != RQUEUE_OK)
            {
                DEBUG(ZONE_ERROR, ("DEMD_Process : Update is failed to LOR @@@@ \r\n"));
            }
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
        if(input != NULL && insize > 0)
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

static bool getIotGPSMessage(char *pdata, int psize, char *pout, int *poutsize)
{
    static char commandbbox[BBOX_MESSAGE_SIZE];
    // parser data and make new message.
    char *cmd = pdata;
    if(strstr((const char *)cmd, (const char*)BBOX_STX_STRING) != NULL) 
    {

        if(psize > BBOX_MESSAGE_SIZE) {
            DEBUG(ZONE_ERROR, ("DEMD_Process : buffer size is larger than buffer. @@@@ (%d)", psize));
        } 
        else
        { 
            DEBUG(ZONE_TRACE, ("DEMD_Process : BBox be arrrived \r\n"));

            // to request data from the gps process.
            reportBBox = true;

            // backup data to make a message after getting gps data.
            memset(commandbbox, 0, BBOX_MESSAGE_SIZE);
            memcpy(commandbbox, cmd, psize);
            GCMD_IOcontrol(GCOM_REPORT_REQUEST, NULL, 0, NULL, 0);
        }

        return false;
    } 
    else if(reportBBox)
    {
        DEBUG(ZONE_TRACE, ("DEMD_Process : GPS be arrrived for BBox \r\n"));
        reportBBox = false;

        parsebbox(commandbbox, BBOX_MESSAGE_SIZE);
        if(isbboxready()) 
        {
            char menufacture = getbboxManufacture();
            char eventcode = getbboxEvent();

            return makeMessageEvent(pdata, psize, menufacture, eventcode, pout, poutsize);
        }
    }
    else 
    {
        return makeMessageGPS(pdata, psize, pout, poutsize );
    }

    return false;
}