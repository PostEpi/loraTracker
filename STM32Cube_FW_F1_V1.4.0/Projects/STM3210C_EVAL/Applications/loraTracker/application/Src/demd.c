#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "demd.h"
#include "vdb.h"
#include "time.h"
#include "timtimer.h"
#include "timeServer.h"
#include "gcommand.h"
#include "nmea.h"
#include "bbox.h"
#include "iotgps.h"
#include "easyflash.h"
#include "lcommand.h"

#define DEMD_PROCESS_LOG_SIZE       4   
#define SETTING_PERIOD_REPORT_CYCLE "period_report_cycle"
#define MIN1                        60000


static TimerEvent_t reportToserverTimer;
static int reportPeriodIndex = 0;
static IOT_MessageTypeDef reportType = IOT_TYPE_NONE;

char log[DEMD_PROCESS_LOG_SIZE*2];
static float prevLatitude = 0.0;
static float prevLongitude = 0.0;

static int getReportCycleFromFlash();
static bool getFinalPositionFromFlash(float *Latitude, float *Longitude);
static bool setFinalPositionToFlash(float Latitude, float Longitude);
static int  getReportTime(int value);
static void OnReportToServerTimer(void);
static bool parseMessage(char *pdata, int psize, char *pout, int *poutsize);
static int  getSecFromDateAndTime();
//static void itohex(char *buf, int size, int value);
static bool storeReportCycle(int value);
static void requestGPSData();


void DEMD_Init()
{
    DEBUG(ZONE_INIT, ("+DEMD_Init\r\n"));
    
    // It brings dat from flash memory to set default value.
    easyflash_init();
#ifdef DEMD_IOT_SK_TEST_SPEC
    reportPeriodIndex = 0;
#else    
    reportPeriodIndex = getReportCycleFromFlash();
#endif    
    getFinalPositionFromFlash(&prevLatitude, &prevLongitude);
    
    // The report timer runs.
    TimerInit(&reportToserverTimer, OnReportToServerTimer);
#ifdef DEMD_IOT_SK_TEST_SPEC
	TimerSetValue(&reportToserverTimer, getReportTime(1)); /* 60000ms */
#else
	TimerSetValue(&reportToserverTimer, getReportTime(reportPeriodIndex)); 
#endif
	TimerStart(&reportToserverTimer);
    
    DEBUG(ZONE_INIT, ("-DEMD_Init prevLatitude=%f pevLongitude=%f\r\n", prevLatitude, prevLongitude));
}

void DEMD_Process()
{
    element item;
    int portready = -1, sizeOfret;
    static char iotMessage[DEMD_IOT_MESSAGE_SIZE];
    static int iotSize = DEMD_IOT_MESSAGE_SIZE;
    
    //DEBUG(ZONE_FUNCTION, ("+DEMD_Process\r\n"));

    if (!isEmptydDB(DEM) && selectDB(DEM, &item) == RQUEUE_OK)
    {
        DEBUG(ZONE_TRACE, ("DEMD_Process : %d, %d, %s\r\n", item.size, item.retcount, item.edata ));
        deleteDB(DEM, &item);

        if(LCMD_IOcontrol(LCOM_MODULE_TX_READY, NULL, 0, &portready, &sizeOfret) == COM_OK)
        { 
            if(portready == 1 && parseMessage(item.edata, item.size, iotMessage, &iotSize))
            {
                // it will send iotMessage to Lora Port. So Lora Port has sent it through Lora network.
                // if sending it is successful, position of iotMessage is stored to flash.
                if(updateDB(LOR, iotMessage, iotSize, 10) != RQUEUE_OK)
                {
                    DEBUG(ZONE_ERROR, ("DEMD_Process : Update is failed to LOR @@@@ \r\n"));
                    return;
                }

                DEBUG(ZONE_FUNCTION, ("DEMD_Process : push datas. %s %d\r\n\r\n", item.edata, item.size));
            }
            else 
            {
                DEBUG(ZONE_FUNCTION, ("\r\n DEMD_Process parseMessage %s %d\r\n\r\n", item.edata, item.size));
            }
        }
        else 
        {
            DEBUG(ZONE_FUNCTION, ("\r\n DEMD_Process : wisol tx is not ready\r\n\r\n"));
        }
    }
    
    //DEBUG(ZONE_FUNCTION, ("-DEMD_Process\r\n"));
}

DEMD_StatusTypeDef DEMD_IOcontrol(DEMD_IOControlTypedef io, int *input, int insize, int *output, int *outsize)
{
    DEMD_StatusTypeDef ret = DEMD_OK;

    DEBUG(ZONE_FUNCTION, ("+DEMD_IOcontrol %d, 0x%x, %d, 0x%x, %d\r\n", io, input, insize, output, *outsize));

    switch (io)
    {
    case DEMD_REPORT_PERIOD_CHANGE:
        // parser for input.
        if(input != NULL && insize > 0)
        {
            if(*input > 255 )
            {
                DEBUG(ZONE_ERROR, ("DEMD_IOcontrol : invalided report time (%d)@@@@\r\n", *input));
                break;
            }

            // periodic time for reporting get to be changed.
            reportPeriodIndex = *input;
            
            storeReportCycle(reportPeriodIndex);
            
            TimerStop(&reportToserverTimer);
            TimerSetValue(&reportToserverTimer, getReportTime(reportPeriodIndex));
            TimerStart(&reportToserverTimer);
        }
        break;
    case DEMD_REPORT_TO_SERVER_FAILED:
        // 
        
        break;
    case DEMD_REPORT_TO_SERVER_SUCCEED:
        setFinalPositionToFlash(prevLatitude, prevLongitude);
        break;    
    default:
        ret = DEMD_PARAM_ERROR;
    }

    DEBUG(ZONE_FUNCTION, ("-DEMD_IOcontrol %d\r\n", ret));
    return ret;
}

static int getReportCycleFromFlash()
{
    int value;
    char *reportenv;
    
    reportenv = ef_get_env(SETTING_PERIOD_REPORT_CYCLE);
    value = atoi(reportenv);
    if(value > 255 )
    {
        value = 1;
    }
    
    DEBUG(ZONE_FUNCTION, ("report cycle %d\r\n", value));

    return value;
}

static bool getFinalPositionFromFlash(float *Latitude, float *Longitude)
{
    //char c_report[10];
              
    /* interger to string */
    //sprintf(c_report,"%3d", reportPeriodIndex);
 
    /* set and store the boot count number to Env */
    //ef_set_env(SETTING_PERIOD_REPORT_CYCLE, c_report);
    //ef_save_env();
    
    if(ef_read_final_log((uint32_t *)log, DEMD_PROCESS_LOG_SIZE + DEMD_PROCESS_LOG_SIZE) != EF_NO_ERR)
    {
        DEBUG(ZONE_ERROR, ("Reading a log on the flash is failed @@@@(%s)\r\n",log));
        *Latitude = 0.0;
        *Longitude = 0.0;

        return false;
      
    }
    DEBUG(ZONE_TRACE, ("read a log on the flash %s\r\n",log));

    // get previous message sent to sever from flash.
    // get the period value 
    // memset(c_report, 0, sizeof(c_report));
    // *Latitude = atof(strncpy(c_report, log, 8));
    // memset(c_report, 0, sizeof(c_report));
    // *Longitude = atof(strncpy(c_report, log+8, 8));
    memcpy(Latitude, log, DEMD_PROCESS_LOG_SIZE);
    memcpy(Longitude, log + DEMD_PROCESS_LOG_SIZE, DEMD_PROCESS_LOG_SIZE);
    return true;
}

static bool setFinalPositionToFlash(float Latitude, float Longitude)
{
//    sprintf(log, "%08f %08f", Latitude, Longitude);
//    DEBUG(ZONE_FUNCTION, ("setFinalPositionToFlash : %s\r\n", log));
    DEBUG(ZONE_FUNCTION, ("setFinalPositionToFlash : %f, %f\r\n", Latitude, Longitude));

    if(ef_log_write((uint32_t *)&Latitude, DEMD_PROCESS_LOG_SIZE) != EF_NO_ERR)
    {
        DEBUG(ZONE_ERROR, ("setFinalPositionToFlash : To store data to flash is failed @@@@ \r\n"));
        return false;
    }
    if(ef_log_write((uint32_t *)&Longitude, DEMD_PROCESS_LOG_SIZE) != EF_NO_ERR)
    {
        DEBUG(ZONE_ERROR, ("setFinalPositionToFlash : To store data to flash is failed @@@@ \r\n"));
        return false;
    }
    
    return true;
}

static int getReportTime(int value)
{
    if(value > 255)
    {
        DEBUG(ZONE_ERROR, ("getReportTime : invalided value (%d)@@@@\r\n", value));
        return MIN1;
    }
#ifdef DEMD_IOT_SK_TEST_SPEC	
    else if(value == 0)
    {
        // To support SK iot spec
        return 10000; // 10 secs

    }
#endif	

    return value*MIN1;
}

static void requestGPSData()
{
    GCMD_IOcontrol(GCOM_REPORT_REQUEST, NULL, 0, NULL, 0);
}

static void OnReportToServerTimer(void)
{
   	requestGPSData();

#ifdef DEMD_IOT_SK_TEST_SPEC
	TimerSetValue(&reportToserverTimer, getReportTime(reportPeriodIndex)); 
#endif
	TimerStart(&reportToserverTimer);
}

static void evaluateGPS(char *pdata, int psize)
{
    int count = psize;
    char *pstr = pdata;
    do
    {
        fusedata(*pstr++);
    } while(count-- > 0);
}

static bool parseMessage(char *pdata, int psize, char *pout, int *poutsize)
{

    static int invalidedRetry = 0;
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
            DEBUG(ZONE_TRACE, ("parseMessage : BBox Message received \r\n"));

            // to request data from the gps process.
            reportType = IOT_TYPE_EVENT;

            // backup data to make a message after getting gps data.
            memset(commandbbox, 0, BBOX_MESSAGE_SIZE);
            memcpy(commandbbox, pdata, psize);
            requestGPSData();
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
                reportType = IOT_TYPE_NONE;
            }
            else
            {
                IotUser_Typedef iotU;
                psize = getUserMessage(commandbbox, psize);

                iotU.type = IOT_TYPE_USER;
                iotU.manufacture = getbboxManufacture();
                memset(iotU.event, 0, sizeof(iotU.event));
                memcpy(iotU.event, commandbbox, psize);

                getIotUserMessage(&iotU, pout, poutsize);
                ret = true;
            }
        }

        return ret;
    }
    else if(reportType == IOT_TYPE_EVENT)
    {
        DEBUG(ZONE_FUNCTION, ("parseMessage : New message will be made by BBox and GPS data\r\n"));

        parsebbox(commandbbox, BBOX_MESSAGE_SIZE);
        if(!isbboxready()) 
        {
            DEBUG(ZONE_ERROR, ("parseMessage : Error : BBox message is not valied @@@@\r\n"));
            reportType = IOT_TYPE_NONE;
            return false;
        }
        
        evaluateGPS(pdata, psize);
        if(isbboxready() && isdataready()) 
        {
            invalidedRetry = 0;

            reportType = IOT_TYPE_NONE;

            IotEvent_Typedef iotE;
            iotE.type = IOT_TYPE_EVENT;
            iotE.manufacture = getbboxManufacture();
            iotE.event = getbboxEvent();
            //itohex(iotE.datetime, 4, getSecFromDateAndTime(getYear(), getMonth(), getDay(), getHour(), getMinute(), getSecond()));
            iotE.datetime = getSecFromDateAndTime(getYear(), getMonth(), getDay(), getHour(), getMinute(), getSecond());
            iotE.latitude = getLatitude();
            iotE.longitude = getLongitude();
            iotE.direction = ((int)getBearing())/3;
            iotE.speed = getSpeed();
            sprintf(&iotE.battery, "%c", getbboxBattery());

            prevLatitude = iotE.latitude;
            prevLongitude = iotE.longitude;  
            
            return getIotEventMessage(&iotE, pout, poutsize);
        }
        else
        {
            // if(invalidedRetry < 5)
            // {
                DEBUG(ZONE_ERROR, ("parseMessage : Error : Bbox & GPS data is not valied. So try to get GPS data again @@@@\r\n"));
                requestGPSData();
            // } 
            // else 
            // {
            //     invalidedRetry = 0;
            //     DEBUG(ZONE_ERROR, ("parseMessage : Error : Bbox & GPS data is not valied.  To get GPS data is failed @@@@\r\n"));
            // }
        }
    }
    else 
    {
        evaluateGPS(pdata, psize);
#ifdef DEMD_IOT_SK_TEST_SPEC  
        if(1)
#else        
        if(isdataready())
#endif
        {
            invalidedRetry = 0;

            IotGPS_Typedef iot;
            iot.type = IOT_TYPE_GPSDATA;
            iot.version = BSP_GetAppVersion();
            sprintf(&iot.reportcycle, "%c", reportPeriodIndex);
            iot.datetime = getSecFromDateAndTime(getYear(), getMonth(), getDay(), getHour(), getMinute(), getSecond());
            iot.latitude = getLatitude();
            iot.longitude = getLongitude();
            iot.direction = ((int)getBearing())/3;
            iot.speed = getSpeed();
            iot.cumulativedistance = GetDistance(iot.latitude, iot.longitude, prevLatitude, prevLongitude);  
            iot.statusofcar = 1;
            
            DEBUG(ZONE_FUNCTION, ("speed = %d, direction=%d\r\n",iot.speed , iot.direction));

            prevLatitude = iot.latitude;
            prevLongitude = iot.longitude;  
            
            return getIotGPSMessage(&iot, pout, poutsize);
        }
        else 
        {
            if(invalidedRetry < 20)
            {
                //DEBUG(ZONE_ERROR, ("demd : parseMessage : Error : GPS data is not valied. So try to get GPS data again @@@@\r\n"));
                //DEBUG(ZONE_ERROR, ("GPS failed @@@@\r\n"));
                requestGPSData();
                invalidedRetry++;
            } 
            else 
            {
                invalidedRetry = 0;
                //DEBUG(ZONE_ERROR, ("demd : parseMessage : Error : GPS data is not valied.  To get GPS data is failed @@@@\r\n"));
                DEBUG(ZONE_ERROR, ("To get GPS data is failed @@@@\r\n"));

            }
        }
    }

    return false;
}

static int getSecFromDateAndTime(int year, int month, int day, int hour, int min, int sec )
{
    char *week[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    time_t user_time;
    struct tm user_stime;
    struct tm *ptr_stime;
    
    DEBUG(ZONE_FUNCTION, ("%d-%d-%d %d:%d:%d\r\n", year, month,  day, hour, min,  sec));

#if 1
//   user_stime.tm_year   = year - 1900;   // cf :year after 1900
   user_stime.tm_year   = year + 100;   // cf :year after 1900
   user_stime.tm_mon    = month      -1;      // cf : mon start at 0
   user_stime.tm_mday   = day;
   user_stime.tm_hour   = hour;
   user_stime.tm_min    = min;
   user_stime.tm_sec    = sec;
   user_stime.tm_isdst  = 0;           // don't use summer time.
#else 
    user_stime.tm_year = 2017 - 1900; 
    user_stime.tm_mon = 6 - 1;        
    user_stime.tm_mday = 20;
    user_stime.tm_hour = 10;
    user_stime.tm_min = 12;
    user_stime.tm_sec = 55;
    user_stime.tm_isdst = 0; 
#endif

    user_time = mktime(&user_stime);
    ptr_stime = localtime(&user_time);

#if 0
    printf("%d sec\r\n", user_time);
    printf("%4d year\r\n", ptr_stime->tm_year + 1900);
    printf("  %2d Mon(0-11)\r\n", ptr_stime->tm_mon + 1);
    printf("  %2d Day(1-31)\r\n", ptr_stime->tm_mday);
    printf("%s\r\n", week[ptr_stime->tm_wday]);
    printf("  %2d H(0-23)\r\n", ptr_stime->tm_hour);
    printf("  %2d M(0-59)\r\n", ptr_stime->tm_min);
    printf("  %2d S(0-59)\r\n", ptr_stime->tm_sec);
    printf("1M 1D after day: %3d \r\n", ptr_stime->tm_yday);

    
    if (0 < ptr_stime->tm_isdst)
        printf("Summer Time on\r\n");
    else if (0 == ptr_stime->tm_isdst)
        printf("Summer Time off\r\n");
    else
        printf("Summer Time disabled\r\n");
#endif
    
    return user_time;
}

static bool storeReportCycle(int value)
{
    char c_report[5];
    
    if(value > 255)
    {
        DEBUG(ZONE_ERROR, ("storeReportCycle : invalided report time (%d)@@@@\r\n", value));
        return false;
    }
    /* interger to string */
    sprintf(c_report, "%3d", value);

    /* set and store the boot count number to Env */
    ef_set_env(SETTING_PERIOD_REPORT_CYCLE, c_report);
    ef_save_env();
    
    return true;
}
