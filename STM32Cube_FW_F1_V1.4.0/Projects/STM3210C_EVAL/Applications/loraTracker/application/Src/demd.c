#include "demd.h"
#include "vdb.h"
#include "timtimer.h"
#include "timeServer.h"
#include "gcommand.h"
#include "bbox.h"
#include "iotgps.h"
#include "easyflash.h"

#define DEMD_PROCESS_LOG_SIZE       16
#define SETTING_PERIOD_REPORT_CYCLE "period_report_cycle"

const int REPORTITME[] = {5000, 600000, 300000, 60000 };


static TimerEvent_t reportToserverTimer;
static int reportPeriodIndex = 0;
static bool reportBBox = false;
static float prevLatitude = 0.0;
static float pevLongitude = 0.0;

static int getReportTime(int value);
static void OnReportToServerTimer(void);
static bool parseMessage(char *pdata, int psize, char *pout, int *poutsize);

void DEMD_Init()
{
    char log[DEMD_PROCESS_LOG_SIZE];
    int value ;
    char c_report[5];
    char *reportenv;
    
    DEBUG(ZONE_INIT, ("+DEMD_Init\r\n"));
    
    easyflash_init();

    reportenv = ef_get_env(SETTING_PERIOD_REPORT_CYCLE);
    value = atoi(reportenv);
    if(value > sizeof(REPORTITME) - 1 || value < 1 )
    {
      value = 1;
    }
    
    reportPeriodIndex = value;
    DEBUG(ZONE_TRACE, ("report cycle %d\r\n", reportPeriodIndex));
 
    reportPeriodIndex = 0;
    DEBUG(ZONE_TRACE, ("report cycle testing ~~~~~ %d\r\n", reportPeriodIndex));
 
                
    /* interger to string */
    //sprintf(c_report,"%ld", reportPeriodIndex);
 
    /* set and store the boot count number to Env */
    //ef_set_env(SETTING_PERIOD_REPORT_CYCLE, c_report);
    //ef_save_env();
    
    ef_read_final_log((uint32_t *)log, DEMD_PROCESS_LOG_SIZE);
    DEBUG(ZONE_TRACE, ("read a log on the flash %s\r\n",log));

    // get previous message sent to sever from flash.
    // get the period value 
    // prevLatitude = 0.0;
    // pevLongitude = 0.0;
    
    TimerInit(&reportToserverTimer, OnReportToServerTimer);
	TimerSetValue(&reportToserverTimer, getReportTime(reportPeriodIndex)); /* 1000ms */
	TimerStart(&reportToserverTimer);
    
    DEBUG(ZONE_INIT, ("-DEMD_Init\r\n"));
}

void DEMD_Process()
{
    char *cmd;
    element item;
    static char iotMessage[DEMD_IOT_MESSAGE_SIZE];
    static int iotSize = DEMD_IOT_MESSAGE_SIZE;
    
    //DEBUG(ZONE_FUNCTION, ("+DEMD_Process\r\n"));

    if (!isEmptydDB(DEM) && selectDB(DEM, &item) == RQUEUE_OK)
    {
        DEBUG(ZONE_TRACE, ("DEMD_Process : %d, %d, %s\r\n", item.size, item.retcount, item.edata ));
        deleteDB(DEM, &item);

        if(parseMessage(item.edata, item.size, iotMessage, &iotSize))
        {
            if(updateDB(LOR, iotMessage, iotSize, true) != RQUEUE_OK)
            {
                DEBUG(ZONE_ERROR, ("DEMD_Process : Update is failed to LOR @@@@ \r\n"));
            }
        }
        else 
        {
#if 1
            DEBUG(ZONE_TRACE, ("\r\n---- push log  %s %d\r\n\r\n", item.edata, item.size));
            ef_log_write((uint32_t *)item.edata, DEMD_PROCESS_LOG_SIZE);
            if(updateDB(LOR, item.edata, item.size, true) != RQUEUE_OK)
            {
                DEBUG(ZONE_ERROR, ("DEMD_Process : Update is failed to LOR @@@@ \r\n"));
            }
#endif            
            
        }

    }
    
    //DEBUG(ZONE_FUNCTION, ("-DEMD_Process\r\n"));
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
            char c_report[5];
            if(*input > sizeof(REPORTITME) - 1 )
            {
                DEBUG(ZONE_ERROR, ("DEMD_IOcontrol : invalided report time (%d)@@@@\r\n", *input));
                break;
            }

            // periodic time for reporting get to be changed.
            reportPeriodIndex = *input;
            
            /* interger to string */
            sprintf(c_report,"%ld", reportPeriodIndex);
            
            /* set and store the boot count number to Env */
            ef_set_env(SETTING_PERIOD_REPORT_CYCLE, c_report);
            ef_save_env();
            
            TimerStop(&reportToserverTimer);
            TimerSetValue(&reportToserverTimer, getReportTime(reportPeriodIndex));
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

static int getReportTime(int value)
{
    if(value > sizeof(REPORTITME) - 1 )
    {
        DEBUG(ZONE_ERROR, ("getReportTime : invalided value (%d)@@@@\r\n", value));
        return REPORTITME[1];
    }

    return REPORTITME[value];
}

static void OnReportToServerTimer(void)
{
   	GCMD_IOcontrol(GCOM_REPORT_REQUEST, NULL, 0, NULL, 0);
	TimerStart(&reportToserverTimer);
}

static bool parseMessage(char *pdata, int psize, char *pout, int *poutsize)
{
    static char commandbbox[BBOX_MESSAGE_SIZE];

    // parser data and make new message.
    char *cmd = pdata;
    if(strstr((const char *)cmd, (const char*)BBOX_STX_STRING) != NULL) 
    {

        if(psize > BBOX_MESSAGE_SIZE) {
            DEBUG(ZONE_ERROR, ("parseMessage : buffer size is larger than buffer. @@@@ (%d)", psize));
        } 
        else
        { 
            DEBUG(ZONE_TRACE, ("parseMessage : BBox Message be arrrived \r\n"));

            // to request data from the gps process.
            reportBBox = true;

            // backup data to make a message after getting gps data.
            memset(commandbbox, 0, BBOX_MESSAGE_SIZE);
            memcpy(commandbbox, pdata, psize);
            GCMD_IOcontrol(GCOM_REPORT_REQUEST, NULL, 0, NULL, 0);
        }

        return false;
    } 
    else if(reportBBox)
    {
        DEBUG(ZONE_TRACE, ("parseMessage : GPS Message be arrrived for BBox \r\n"));
        reportBBox = false;

        parsebbox(commandbbox, BBOX_MESSAGE_SIZE);
        fusedata(pdata, psize);
        if(isbboxready() && isdataready()) 
        {
            IotGPS_Typedef iot;
            iot.manufacture = getbboxManufacture();
            iot.event = getbboxEvent();
            sprintf(iot.datetime, "%02c%02c%02c%02c", getDay(), getHour(), getMinute(), getSecond());
            iot.latitude = getLatitude();
            iot.longitude = getLongitude();
            iot.direction = getBearing();
            iot.speed = getSpeed();
            iot.battordistance = getbboxBattery();

            prevLatitude = iot.latitude;
            pevLongitude = iot.longitude;  
            
            return getIotGPSMessage(&iot, pout, poutsize);
        }
    }
    else 
    {
        fusedata(pdata, psize);
        if(isdataready()) 
        {
            IotGPS_Typedef iot;
            iot.manufacture = BSP_GetAppVersion();
            sprintf(&iot.event, "%c", reportPeriodIndex);
            sprintf(iot.datetime, "%02c%02c%02c%02c", getDay(), getHour(), getMinute(), getSecond());
            iot.latitude = getLatitude();
            iot.longitude = getLongitude();
            iot.direction = getBearing();
            iot.speed = getSpeed();
            iot.battordistance = GetDistance(iot.latitude, iot.longitude, prevLatitude, pevLongitude);  
            
            prevLatitude = iot.latitude;
            pevLongitude = iot.longitude;  
            
            return getIotGPSMessage(&iot, pout, poutsize);
        }
    }

    return false;
}