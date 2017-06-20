#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "demd.h"
#include "vdb.h"
#include "time.h"
#include "timtimer.h"
#include "timeServer.h"
#include "gcommand.h"
#include "bbox.h"
#include "iotgps.h"
#include "easyflash.h"

#define DEMD_PROCESS_LOG_SIZE       16
#define SETTING_PERIOD_REPORT_CYCLE "period_report_cycle"

const int REPORTITME[] = {120000, 600000, 300000, 60000 };



static TimerEvent_t reportToserverTimer;
static int reportPeriodIndex = 0;
static BBOX_MessageTypeDef reportBBox = BBOX_NONE_MESSAGE;

char log[DEMD_PROCESS_LOG_SIZE];
static float prevLatitude = 0.0;
static float pevLongitude = 0.0;

static int getReportTime(int value);
static void OnReportToServerTimer(void);
static bool parseMessage(char *pdata, int psize, char *pout, int *poutsize);
static int getSecFromDateAndTime();
static void itohex(char *buf, int size, int value);

void DEMD_Init()
{

    int value ;
    char c_report[10];
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
    memset(c_report, 0, sizeof(c_report));
    prevLatitude = atof(strncpy(c_report, log, 8));
    memset(c_report, 0, sizeof(c_report));
    pevLongitude = atof(strncpy(c_report, log+8, 8));
    // prevLatitude = 0.0;
    // pevLongitude = 0.0;
    
    TimerInit(&reportToserverTimer, OnReportToServerTimer);
	TimerSetValue(&reportToserverTimer, getReportTime(reportPeriodIndex)); /* 1000ms */
	TimerStart(&reportToserverTimer);
    
    DEBUG(ZONE_INIT, ("-DEMD_Init prevLatitude=%f pevLongitude=%f\r\n", prevLatitude, pevLongitude));
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
                return;
            }

            sprintf(log, "%08f%08f", prevLatitude, pevLongitude);
            DEBUG(ZONE_FUNCTION, ("DEMD_Process : %s\r\n", log));
            ef_log_write((uint32_t *)log, DEMD_PROCESS_LOG_SIZE);

        }
        else 
        {
            DEBUG(ZONE_FUNCTION, ("\r\n DEMD_Process parseMessage %s %d\r\n\r\n", item.edata, item.size));
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
    if(strstr((const char *)cmd, (const char*)BBOX_STX_BERDP) != NULL) 
    {
        if(psize > BBOX_MESSAGE_SIZE) {
            DEBUG(ZONE_ERROR, ("parseMessage : buffer size is larger than buffer. @@@@ (%d)", psize));
        } 
        else
        { 
            DEBUG(ZONE_TRACE, ("parseMessage : BBox Message be arrrived \r\n"));

            // to request data from the gps process.
            reportBBox = BBOX_ENVET_MESSAGE;

            // backup data to make a message after getting gps data.
            memset(commandbbox, 0, BBOX_MESSAGE_SIZE);
            memcpy(commandbbox, pdata, psize);
            GCMD_IOcontrol(GCOM_REPORT_REQUEST, NULL, 0, NULL, 0);
        }

        return false;
    } 
    else if(strstr((const char *)cmd, (const char*)BBOX_STR_BURDP) != NULL)
    {
        bool ret = false;
         
        if(psize > BBOX_MESSAGE_SIZE) {
            DEBUG(ZONE_ERROR, ("parseMessage : buffer size is larger than buffer. @@@@ (%d)", psize));
        } 
        else
        { 
            DEBUG(ZONE_TRACE, ("parseMessage : User Message from BBox has been arrrived \r\n"));

            // to request data from the gps process.
            parsebbox(cmd, psize);
            if(!isbboxready()) 
            {
                DEBUG(ZONE_ERROR, ("parseMessage : Error : BBox message is not valied @@@@\r\n"));
                reportBBox = BBOX_NONE_MESSAGE;
            }
            else
            {
                psize = getUserMessage(commandbbox, psize);
                getIotUserMessage(commandbbox, psize, pout, poutsize);
                ret = true;
            }
        }

        return ret;
    }
    else if(reportBBox == BBOX_ENVET_MESSAGE)
    {
        DEBUG(ZONE_FUNCTION, ("parseMessage : New message will be made by BBox and GPS data\r\n"));

        parsebbox(commandbbox, BBOX_MESSAGE_SIZE);
        if(!isbboxready()) 
        {
            DEBUG(ZONE_ERROR, ("parseMessage : Error : BBox message is not valied @@@@\r\n"));
            reportBBox = BBOX_NONE_MESSAGE;
            return false;
        }

        fusedata(pdata, psize);
        if(isbboxready() && isdataready()) 
        {
            int i = 0;
            reportBBox = BBOX_NONE_MESSAGE;

            IotGPS_Typedef iot;
            iot.type = IOTGPS_TYPE_EVENT;
            iot.manufacture = getbboxManufacture();
            iot.event = getbboxEvent();
            //itohex(iot.datetime, 4, getSecFromDateAndTime(getYear(), getMonth(), getDay(), getHour(), getMinute(), getSecond()));
            iot.datetime = getSecFromDateAndTime(getYear(), getMonth(), getDay(), getHour(), getMinute(), getSecond());
            iot.latitude = getLatitude();
            iot.longitude = getLongitude();
            iot.direction = (int)getBearing()/3;
            iot.speed = getSpeed();
            iot.battordistance = getbboxBattery();

            prevLatitude = iot.latitude;
            pevLongitude = iot.longitude;  
            
            return getIotGPSMessage(&iot, pout, poutsize);
        }
        else
        {
            // 
            DEBUG(ZONE_ERROR, ("parseMessage : Error : GPS data is not valied. So try to get GPS data again @@@@\r\n"));
            GCMD_IOcontrol(GCOM_REPORT_REQUEST, NULL, 0, NULL, 0);
        }
    }
    else 
    {
        fusedata(pdata, psize);
        if(isdataready())
        {
            IotGPS_Typedef iot;
            iot.type = IOTGPS_TYPE_GPSDATA;
            iot.manufacture = BSP_GetAppVersion();
            sprintf(&iot.event, "%c", reportPeriodIndex);
            iot.datetime = getSecFromDateAndTime(getYear(), getMonth(), getDay(), getHour(), getMinute(), getSecond());
            iot.latitude = getLatitude();
            iot.longitude = getLongitude();
            iot.direction = (int)getBearing()/3;
            iot.speed = getSpeed();
            iot.battordistance = GetDistance(iot.latitude, iot.longitude, prevLatitude, pevLongitude);  
            
            prevLatitude = iot.latitude;
            pevLongitude = iot.longitude;  
            
            return getIotGPSMessage(&iot, pout, poutsize);
        }
    }

    return false;
}




static int getSecFromDateAndTime(int year, int month, int day, int hour, int min, int sec )
{
   char      *week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
   time_t     user_time;
   struct tm  user_stime;
   struct tm *ptr_stime;

#if 0
   user_stime.tm_year   = year   -1900;   // cf :year after 1900
   user_stime.tm_mon    = month      -1;      // cf : mon start at 0
   user_stime.tm_mday   = day;
   user_stime.tm_hour   = hour;
   user_stime.tm_min    = min;
   user_stime.tm_sec    = sec;
   user_stime.tm_isdst  = 0;           // don't use summer time.
#endif
   
   user_stime.tm_year   = 2017   -1900;   // 주의 :년도는 1900년부터 시작
   user_stime.tm_mon    = 6      -1;      // 주의 :월은 0부터 시작
   user_stime.tm_mday   = 20;
   user_stime.tm_hour   = 10;
   user_stime.tm_min    = 12;
   user_stime.tm_sec    = 55;
   user_stime.tm_isdst  = 0;           // 썸머 타임 사용 안함
   
   user_time   = mktime( &user_stime);
   ptr_stime = localtime( &user_time);

   printf( "%d sec\r\n"        , user_time);
   printf( "%4d year\r\n"        , ptr_stime->tm_year +1900);
   printf( "  %2d Mon(0-11)\r\n", ptr_stime->tm_mon  +1   );
   printf( "  %2d Day(1-31)\r\n", ptr_stime->tm_mday      );
   printf( "%s\r\n"        , week[ptr_stime->tm_wday]);
   printf( "  %2d H(0-23)\r\n", ptr_stime->tm_hour      );
   printf( "  %2d M(0-59)\r\n", ptr_stime->tm_min       );
   printf( "  %2d S(0-59)\r\n", ptr_stime->tm_sec       );
   printf( "1M 1D after day: %3d \r\n", ptr_stime->tm_yday);
   if      ( 0 <  ptr_stime->tm_isdst)    printf( "Summer Time on\r\n"     );
   else if ( 0 == ptr_stime->tm_isdst)    printf( "Summer Time off\r\n");
   else                                   printf( "Summer Time disabled\r\n");

   return user_time;
}