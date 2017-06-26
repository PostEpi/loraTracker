#include <stdlib.h>
#include "stm32f1xx.h"
#include "wisol.h"
#include "time.h"
#include "timtimer.h"
#include "timeServer.h"
#include "demd.h"
#include "debug.h"

#define WISOL_MAX_RESPONSE_TIME     50000
#define WISOL_RESPONSE_TIME         3000
#define WISOL_FIRST_CMD_TIME        3000
#define WISOL_BUFFER_SIZE           65
#define WISOL_USER_DATA_LIMIT       55

typedef enum{
    NONE,
    UNCONFIRM,
    CONFIRM
}Wisol_ResposeTypeDef;

static TimerEvent_t respExpireWisolTimer;
static TimerEvent_t respWisolTimer;
static TimerEvent_t cmdWisolTimer;

static bool wisolInit = false;
static bool waitAck = false;
static Wisol_ResposeTypeDef response;
static int Wisol_reboot = 0;
static int Wisol_respError = 0;
static callbackFunc callback = NULL;
static char lrwcmd[WISOL_BUFFER_SIZE];
static int lrwsize = WISOL_BUFFER_SIZE;

static void wakeup();
static void OnCmdFirst();
static void OnExpire();
static void OnResponse();
static void cmdTimerOn();
static bool sendTx(const char *msg, int size, Wisol_ResposeTypeDef resptype);
static bool sendBinaryTx(const char *msg, int size, Wisol_ResposeTypeDef resptype);
static bool send(const char *cmd, int size);

void initWisol(callbackFunc cb)
{
    callback = cb;
    Wisol_reboot = 0;
    wisolInit = false;

    TimerInit(&cmdWisolTimer, OnCmdFirst);
    TimerInit(&respWisolTimer, OnResponse);
    TimerInit(&respExpireWisolTimer, OnExpire);

    // To check Lora's port. 
    memset(lrwcmd, 0, WISOL_BUFFER_SIZE);
    strcpy(lrwcmd, "LRW 50 0\r\n");
    lrwsize = strlen(lrwcmd); 
    cmdTimerOn();
}

bool writeLRW(const char *msg, int size)
{
    if(wisolInit) 
    {
        if(size < WISOL_USER_DATA_LIMIT && !waitAck) 
        {
            waitAck = true;
            Wisol_respError = 0;
            response = NONE;
            return sendBinaryTx(msg, size, CONFIRM);
        }
        else 
        {
            if(size >= WISOL_USER_DATA_LIMIT) 
            {
                DEBUG(ZONE_ERROR, ("The size of message is larger than the content that the LORA can transmit \r\n"));;    
            }
            else if(waitAck)
            {
                DEBUG(ZONE_ERROR, ("In sending Message @@@@ \r\n"));;
            }
        }
    }

    return false;
}


static void wakeup()
{
    BSP_Lora_Wakeup();
}

static void reset()
{
    BSP_Lora_HW_Reset();
}


static void OnCmdFirst()
{
    send(lrwcmd, lrwsize);
}

static void OnExpire()
{

}

static void OnResponse()
{
    // if there is no reponse until limit itme, lora module may be dead.
    DEBUG(ZONE_ERROR, ("Lora module will need to start over @@@@ \r\n"));;
    wisolInit = false;
    reset();
    

    // the command stored in buffer is retransmited after Lora has been reset.
    cmdTimerOn();
}

static void cmdTimerOn()
{
	TimerSetValue(&cmdWisolTimer, WISOL_FIRST_CMD_TIME);
	TimerStart(&cmdWisolTimer);
}


static void responseTimerOn()
{
	TimerSetValue(&respWisolTimer, WISOL_MAX_RESPONSE_TIME);
	TimerStart(&respWisolTimer);
}

static void responseTimerOff()
{
	TimerSetValue(&respWisolTimer, WISOL_MAX_RESPONSE_TIME);
	TimerStart(&respWisolTimer);
}

static void responseExpireTimerON()
{

	TimerSetValue(&respExpireWisolTimer, WISOL_MAX_RESPONSE_TIME);
	TimerStart(&respExpireWisolTimer);
}

static bool sendTx(const char *msg, int size, Wisol_ResposeTypeDef resptype)
{
    if(size > WISOL_BUFFER_SIZE - 12)
    {
        DEBUG(ZONE_ERROR, ("sendTx : The size of message is larger than the content\r\n"));;    
        return false;
    }

    response = resptype;
    memset(lrwcmd, 0, WISOL_BUFFER_SIZE);

    send(msg, size);
}

static bool sendBinaryTx(const char *msg, int size, Wisol_ResposeTypeDef resptype)
{
    if(size > WISOL_BUFFER_SIZE - 12)
    {
        DEBUG(ZONE_ERROR, ("sendBinaryTx : The size of message is larger than the content\r\n"));;    
        return false;
    }

    response = resptype;
    memset(lrwcmd, 0, WISOL_BUFFER_SIZE);
    char *cmd = lrwcmd;
    
#if 1
    *(cmd++) = 'L';
    *(cmd++) = 'R';
    *(cmd++) = 'W';
    *(cmd++) = ' ';
    *(cmd++) = '4';
    *(cmd++) = 'D';
    *(cmd++) = ' ';
    
    // Mtype
    if(resptype == CONFIRM)
        *(cmd++) = 0;
    else
        *(cmd++) = 1;

    // Fport
    *(cmd++) = 1;
    
    // length
    *(cmd++) = size;
#endif    
    // Message
    memcpy(cmd, msg, size);

    *(cmd+size) = '\r';      // CR
    *(cmd+size+1) = '\n';    // LF

    lrwsize = 12 + size;

    send(lrwcmd, lrwsize);
}

static bool send(const char *cmd, int size)
{
    wakeup();

    if(callback != NULL)
    {
        (*callback)(cmd, size);
        responseTimerOn();
        responseExpireTimerON();
    }
}

bool parser_Wisol(const char *cmd, int size)
{
    wisolInit = true;

    if(cmd[0] == 10) 
    {
        int reporttime = cmd[1];
        DEBUG(ZONE_TRACE, ("Reporting cycle will change to %d\r\n", reporttime));
        DEMD_IOcontrol(DEMD_REPORT_PERIOD_CHANGE, &reporttime, 1, NULL, 0);
    }

    return false;
}