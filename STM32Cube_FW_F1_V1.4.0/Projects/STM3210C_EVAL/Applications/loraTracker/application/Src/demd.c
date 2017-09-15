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
#include "carstatus.h"

#define DEMD_PROCESS_LOG_SIZE       4   
#define SETTING_PERIOD_REPORT_CYCLE "period_report_cycle"
#define SETTING_APPKEY_ACTIVATION   "appkey_activation"
#define MIN1                        60000


static TimerEvent_t reportToserverTimer;
static int reportPeriodIndex = 0;
static bool appkeyactivation = true;
static IOT_MessageTypeDef reportType = IOT_TYPE_NONE;
static bool proceesOnByTimer = false;

char log[DEMD_PROCESS_LOG_SIZE*2];
static float prevLatitude = 0.0;
static float prevLongitude = 0.0;
static int prevSpeed = 0;
static bool carStatusStop = false;

static int getReportCycleFromFlash();
static bool getFinalPositionFromFlash(float *Latitude, float *Longitude);
static bool setFinalPositionToFlash(float Latitude, float Longitude);
static int  getReportTime(int value);
static bool IsAPPKeyWrittenToFlash();
static void OnDemdTimerEvent(void);
static void demdProcessTimerStart(int value);
static bool parseMessage(char *pdata, int psize, char *pout, int *poutsize);
static int  getSecFromDateAndTime();
//static void itohex(char *buf, int size, int value);
static bool storeReportCycle(int value);
static bool storeAppKeyActivated(bool bactive);
static void requestGPSData();


void DEMD_Init()
{
#ifdef DEMO_IOT_RF_TX_MODE_FOR_MASSPRODUCT
    DEBUG(ZONE_INIT, ("+DEMD_Init DEMO_IOT_RF_TX_MODE_FOR_MASSPRODUCT\r\n"));
#endif
#ifdef DEMO_IOT_RF_RX_MODE_FOR_MASSPRODUCT
    DEBUG(ZONE_INIT, ("+DEMD_Init DEMO_IOT_RF_RX_MODE_FOR_MASSPRODUCT\r\n"));
#endif
#ifdef DEMO_IOT_DONTCARE_GPSDATA
    DEBUG(ZONE_INIT, ("+DEMD_Init DEMO_IOT_DONTCARE_GPSDATA\r\n"));
#endif
#ifdef DEMD_IOT_SK_PREPARE_SPEC
    DEBUG(ZONE_INIT, ("+DEMD_Init DEMD_IOT_SK_PREPARE_SPEC\r\n"));
#endif

#ifdef DEMD_IOT_SK_10SECONDS_TX_SPEC
    DEBUG(ZONE_INIT, ("+DEMD_Init DEMD_IOT_SK_10SECONDS_TX_SPEC\r\n"));
#else
    DEBUG(ZONE_INIT, ("+DEMD_Init\r\n"));
#endif    
    // It brings dat from flash memory to set default value.
    easyflash_init();
    reportPeriodIndex = getReportCycleFromFlash();
    getFinalPositionFromFlash(&prevLatitude, &prevLongitude);
    appkeyactivation = IsAPPKeyWrittenToFlash();

    //storeReportCycle(1);
	// If the server sets the transmission period to 255, iotGPS should not transmit data.
    if(reportPeriodIndex == 255)
    {
        DEBUG(ZONE_INIT, ("-DEMD_Init : DEMD process is disabled\r\n"));    
        return;  
    }

    // The report timer runs.
    TimerInit(&reportToserverTimer, OnDemdTimerEvent);
    demdProcessTimerStart(reportPeriodIndex);

    // roadpia specification.
    CarStatus_Init();
    
    DEBUG(ZONE_INIT, ("-DEMD_Init prevLatitude=%f pevLongitude=%f\r\n", prevLatitude, prevLongitude));
}

void DEMD_Process()
{
    element item;
    int loraReady = -1, sizeOfret = 1;
    static char iotMessage[DEMD_IOT_MESSAGE_SIZE];
    int iotSize = DEMD_IOT_MESSAGE_SIZE;
    
    if(reportPeriodIndex == 255)
    {
        // if it's 255,  the process of data transmission to the server is stopped.
        // But black box may try to pass the black box information to the server, 
        // So if there is data in the database, remove it to prevent buffer overflow.
        if (!isEmptydDB(DEM) && selectDB(DEM, &item) == RQUEUE_OK)
        {
            DEBUG(ZONE_TRACE, ("DEMD_Process : remove data in DB in 255\r\n"));
            deleteDB(DEM, &item);
        }
    
        return;  
    }

    //DEBUG(ZONE_FUNCTION, ("+DEMD_Process\r\n"));

#ifndef DEMD_NO_PROCESS_BY_TIMER
    // The process of DEMOD has to be scheduled by timer setting.
    if(proceesOnByTimer == false && reportPeriodIndex)
        return;

#endif
    
    if (!isEmptydDB(DEM) && selectDB(DEM, &item) == RQUEUE_OK)
    {
#ifndef DEMD_NO_PROCESS_BY_TIMER
        proceesOnByTimer = false;
#endif

        DEBUG(ZONE_TRACE, ("DEMD_Process : %d, %d, %s\r\n", item.size, item.retcount, item.edata ));
        deleteDB(DEM, &item);
        
#ifdef DEMO_IOT_RF_TX_MODE_FOR_MASSPRODUCT
        return;
#endif
#ifdef DEMO_IOT_RF_RX_MODE_FOR_MASSPRODUCT
        return;
#endif

        if(parseMessage(item.edata, item.size, iotMessage, &iotSize))
        {
            // is Lora ready to be able to send a message?
            if(LCMD_IOcontrol(LCOM_MODULE_TX_READY, NULL, 0, &loraReady, &sizeOfret) == COM_OK)
            { 
                if(loraReady == 1 )
                {
                    // it will send iotMessage to Lora Port. So Lora Port has sent it through Lora network.
                    // if sending it is successful, position of iotMessage is stored to flash.
                    if(updateDB(LOR, iotMessage, iotSize, 10) != RQUEUE_OK)
                    {
                        DEBUG(ZONE_ERROR, ("DEMD_Process : Update is failed to LOR @@@@ \r\n"));
                        return;
                    }

                    DEBUG(ZONE_TRACE, ("DEMD_Process : push datas. %s %d\r\n\r\n", item.edata, item.size));
                }
                else 
                {
                    DEBUG(ZONE_WARN, ("\r\n DEMD_Process : wisol tx is not ready.\n\r\n"));
                }
            }
        }
    }
    
    //DEBUG(ZONE_FUNCTION, ("-DEMD_Process\r\n"));
}
static void encryptData(float *prevLatitude, float *prevLongitude)
{

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
            if(*input > 255  && *input < 0)
            {
                DEBUG(ZONE_ERROR, ("DEMD_IOcontrol : invalided report time (%d)@@@@\r\n", *input));
                break;
            }

            DEBUG(ZONE_FUNCTION, ("The cycle reporting has been changed to %d min\r\n", *input));

            // To change reporting cycle time.
            reportPeriodIndex = *input;
            
            // To store it to flash memory.
            storeReportCycle(reportPeriodIndex);

            // To change report timer.
            demdProcessTimerStart(reportPeriodIndex); 
        }
        break;
    case DEMD_REPORT_GET_PERIOD:
        if(output != NULL && *outsize > 0) 
        {
            //DEBUG(ZONE_FUNCTION, ("DEMD_IOcontrol : Longitude=%f\r\n", prevLongitude));
            *output = reportPeriodIndex;
        }
        break;
    case DEMD_REPORT_TO_SERVER_FAILED:
        //If the device fails to send data when it is set to send data to the server every 1 minute, 
        //then the next message should be set to be sent at least 10 seconds later.
        //The transmission interval between messages sent to the server must differ by at least 10 seconds.
        if(reportPeriodIndex == 1) 
        {
#ifndef DEMD_NO_PROCESS_BY_TIMER
            proceesOnByTimer = false;
#endif

            TimerStop(&reportToserverTimer);
            TimerSetValue(&reportToserverTimer, 11000); 
            TimerStart(&reportToserverTimer);
        }
        break;
    case DEMD_REPORT_TO_SERVER_SUCCEED:
        // if speed is smaller than 5km, data is stored into flash memory.
        if(prevSpeed < 5)
        {
            encryptData(&prevLatitude, &prevLongitude); 
            setFinalPositionToFlash(prevLatitude, prevLongitude);
        }

        break;
    case DEMD_REPORT_NETWORK_READY:
        if(reportPeriodIndex == 0) 
        {
            DEBUG(ZONE_FUNCTION, ("DEMD_IOcontrol : dummy data will be transfered\r\n"));

            demdProcessTimerStart(1);
        }
        else if(reportPeriodIndex > 2) 
        {
            DEBUG(ZONE_FUNCTION, ("DEMD_IOcontrol : If the cycle transmission is more than 2 minutes, notify the server of the departure time\r\n"));
            demdProcessTimerStart(1);
        }
        break; 
    case DEMD_REPORT_PREV_GPS:
        if (output != NULL && *outsize == sizeof(DEMD_GPSTypeDef))
        {
            DEMD_GPSTypeDef *gpsdef = (DEMD_GPSTypeDef *)output;
            gpsdef->latitude = prevLatitude;
            gpsdef->longitude = prevLongitude;
        }
        break; 
    case DEMD_REPORT_GET_ACTIVATION_STATUS:
        if (output != NULL && *outsize > 0)
        {
            *output = (appkeyactivation == true)? 1 : 0;
            *outsize = 1;
        }
        break;
    default:
        ret = DEMD_PARAM_ERROR;
    }

    DEBUG(ZONE_FUNCTION, ("-DEMD_IOcontrol %d\r\n", ret));
    return ret;
}

void evaluateGPS(char *pdata, int psize)
{
    int count = psize;
    char *pstr = pdata;
    do
    {
        fusedata(*pstr++);
    } while(count-- > 0);
}


static int getReportCycleFromFlash()
{
    int value;
    char *reportenv;
    
    reportenv = ef_get_env(SETTING_PERIOD_REPORT_CYCLE);
    value = atoi(reportenv);
    if(value > 255 )
    {
        value = 10;
    }
    
    DEBUG(ZONE_FUNCTION, ("report cycle %d\r\n", value));

    return value;
}


static bool IsAPPKeyWrittenToFlash()
{
    char *appkeyenv;
    
    appkeyenv = ef_get_env(SETTING_APPKEY_ACTIVATION);
    
    if(appkeyenv == "1")
    {
        DEBUG(ZONE_FUNCTION, ("Activation on\r\n"));
    }
    else
        DEBUG(ZONE_FUNCTION, ("Not Activation on\r\n"));
        
    return (appkeyenv == "1")? true:false;
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

static int getReportTime(int valueofMin)
{
    if(valueofMin > 255)
    {
        DEBUG(ZONE_ERROR, ("getReportTime : invalided value (%d)@@@@\r\n", valueofMin));
        return MIN1;
    }
#ifdef DEMD_IOT_SK_10SECONDS_TX_SPEC	
 
        // To support SK iot spec
        return 10000; // 10 secs
#endif

    return valueofMin*MIN1;
}

static void dummyCommand() 
{
    // This is used to get the data sent from the thingplug when the perioded timer is 0.
    char msg[DEMD_IOT_MESSAGE_SIZE];
    int size = DEMD_IOT_MESSAGE_SIZE;
    
    IotGPS_Typedef iot;
    memset(&iot, 0, sizeof(IotGPS_Typedef));

    iot.type = IOT_TYPE_DUMMY;
    iot.version = BSP_GetAppVersion();
    sprintf(&iot.reportcycle, "%c", reportPeriodIndex);
    
    if(getIotGPSMessage(&iot, msg, &size)) 
    {
        if(updateDB(LOR, msg, size, 10) != RQUEUE_OK)
        {
            DEBUG(ZONE_ERROR, ("DEMD_IOcontrol : Update is failed to LOR @@@@ \r\n"));
        }
    }
}

static void requestGPSData()
{
#ifndef DEMD_NO_PROCESS_BY_TIMER
    proceesOnByTimer = true;
#endif

    GCMD_IOcontrol(GCOM_REPORT_REQUEST, NULL, 0, NULL, 0);
}

static void OnDemdTimerEvent(void)
{
    if(reportPeriodIndex == 255)
    {
        DEBUG(ZONE_ERROR, ("OnDemdTimerEvent : This prevent iotGPS from transmitting a data to server @@@@ \r\n"));
        return ;
    }

    if(reportPeriodIndex)
       requestGPSData();
    else
       dummyCommand();
   	
    demdProcessTimerStart(reportPeriodIndex);
}

static void demdProcessTimerStart(int valueofMin)
{
    if(valueofMin == 0 || valueofMin == 255)
    {
        TimerStop(&reportToserverTimer);
    }
    else
    {
        TimerSetValue(&reportToserverTimer, getReportTime(valueofMin)); 
        TimerStart(&reportToserverTimer);
    }
}


static bool parseMessage(char *pdata, int psize, char *pout, int *poutsize)
{

    static int invalidedRetry = 0;
    static char commandbbox[BBOX_MESSAGE_SIZE];

    // parser data and make new message.
    char *cmd = pdata;
    // if(strstr((const char *)cmd, (const char*)BBOX_STX_BERDP) != NULL) 
    if(strstr((const char *)cmd, (const char*)BBOX_STR_BURDP) != NULL)
    {
        if(psize > BBOX_MESSAGE_SIZE) {
            DEBUG(ZONE_ERROR, ("parseMessage : buffer size is larger than buffer. @@@@ (%d)", psize));
        } 
        else
        { 
            DEBUG(ZONE_TRACE, ("parseMessage : BBox Message received \r\n"));
            
            // If there is no periodic reporting, send the data coming from the black box directly. 
            if(reportPeriodIndex == 0)
            {
                parsebbox(pdata, psize);
                if(isbboxready()) 
                {
                    invalidedRetry = 0;

                    reportType = IOT_TYPE_NONE;

                    IotEvent_Typedef iotE;
                    iotE.type = IOT_TYPE_EVENT;
                    iotE.serviceversion = getServiceCode();
                    iotE.manufacture = getbboxManufacture();
                    iotE.event = getbboxEvent();
                    iotE.eventinfo = getbboxEventInfo();
                    iotE.battery = getbboxBattery();         
                    iotE.temperature =  getbboxTemperature();
                    memcpy(iotE.boxserial, (void const *)getbboxSerial(), MAX_SIZE_OF_BBOX_COLUME);
                    iotE.datetime = 0;
                    iotE.latitude = 0;
                    iotE.longitude = 0;
                    iotE.direction = 0;
                    iotE.speed = 0;
        
                    prevSpeed = 0;
                    prevLatitude = 0.0;
                    prevLongitude = 0.0;  
                    
                    return getIotEventMessage(&iotE, pout, poutsize);
                }
            } 
            else 
            {
                // to request data from the gps process.
                reportType = IOT_TYPE_EVENT;

                // backup data to make a message after getting gps data.
                memset(commandbbox, 0, BBOX_MESSAGE_SIZE);
                memcpy(commandbbox, pdata, psize);
                requestGPSData();
            }
        }

        return false;
    }
#if 0     
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
#endif     
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

#ifdef DEMO_IOT_DONTCARE_GPSDATA
        if (1)
#else
        if (isdataready())
#endif
        {
            invalidedRetry = 0;

            reportType = IOT_TYPE_NONE;

            IotEvent_Typedef iotE;
            iotE.type = IOT_TYPE_EVENT;
            iotE.serviceversion = getServiceCode();
            iotE.manufacture = getbboxManufacture();
            iotE.event = getbboxEvent();
            iotE.eventinfo = getbboxEventInfo();
            iotE.battery = getbboxBattery();         
            iotE.temperature =  getbboxTemperature();
            memcpy(iotE.boxserial, (void const *)getbboxSerial(), MAX_SIZE_OF_BBOX_COLUME);
            iotE.datetime = getSecFromDateAndTime(getYear(), getMonth(), getDay(), getHour(), getMinute(), getSecond());
            iotE.latitude = getIntLatitude();
            iotE.longitude = getIntLongitude();
            iotE.direction = ((int)getBearing())/IOT_GPS_SPECIFICATION;
            iotE.speed = getSpeed();
   
            prevSpeed = getSpeed();
            prevLatitude = getLatitude();
            prevLongitude = getLongitude();  
            
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
#ifdef DEMD_IOT_SK_10SECONDS_TX_SPEC  
        if(1)
#else
 #ifdef DEMO_IOT_DONTCARE_GPSDATA
        if(1)
 #else        
        if(isdataready())
 #endif        
#endif
        {
            int distance, size;
            invalidedRetry = 0;

            IotGPS_Typedef iot;
            iot.type = IOT_TYPE_GPSDATA;
            iot.version = BSP_GetAppVersion();
            sprintf(&iot.reportcycle, "%c", reportPeriodIndex);
            iot.datetime = getSecFromDateAndTime(getYear(), getMonth(), getDay(), getHour(), getMinute(), getSecond());
            iot.latitude = getIntLatitude();
            iot.longitude = getIntLongitude();
            iot.direction = ((int)getBearing())/IOT_GPS_SPECIFICATION;
            iot.speed = getSpeed();

            size = 1;
            GCMD_IOcontrol(GCOM_REPORT_DISTANCE_BETWEEN_GPS, NULL, 0, &distance, &size);
            iot.cumulativedistance = (short)distance;
            iot.statusofcar = getCarStatus(iot.speed, iot.speed);
          
#ifndef DEMD_IOT_SK_10SECONDS_TX_SPEC
            // IotGPS-blackbox spec
            // if vehicle is in parking, no more reporting anymore after parking status is sent to server.
            // if status of vehicle may be changed, recover periodic transmission.
            if(iot.statusofcar == CAR_STATUS_STOPING ) 
            {
                if(carStatusStop)
                {
                    DEBUG(ZONE_FUNCTION, ("Vehicle parking status. No more reporting anymore.\r\n",iot.speed , iot.direction));
                    return false;
                }
                carStatusStop = true;
            }
            else 
                carStatusStop == false;
#endif

            prevSpeed = getSpeed();
            prevLatitude = getLatitude();
            prevLongitude = getLongitude();  
            
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
    
    DEBUG(ZONE_TRACE, ("%d-%d-%d %d:%d:%d\r\n", year, month,  day, hour, min,  sec));

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
    //user_time += 32400; // korea time zone.

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

static bool storeAppKeyActivated(bool bactive)
{
    DEBUG(ZONE_FUNCTION, ("storeAppKeyActivated : Activation is %d\r\n", bactive));

    char c_report[2];
    memset(c_report, 0, sizeof(c_report));
    if(bactive)
        strcpy(c_report, "1");
    else 
        strcpy(c_report, "0");

    /* set and store the boot count number to Env */
    ef_set_env(SETTING_APPKEY_ACTIVATION, c_report);
    ef_save_env();
    
    return true;
}
