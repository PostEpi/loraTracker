#include <stdlib.h>
#include "stm32f1xx.h"
#include "wisol.h"
#include "time.h"
#include "timtimer.h"
#include "timeServer.h"
#include "demd.h"
#include "lcommand.h"
#include "debug.h"

#define WISOL_MAX_RESPONSE_TIME     50000
#define WISOL_RESPONSE_TIME         10000
#define WISOL_BUFFER_SIZE           65
#define WISOL_USER_DATA_LIMIT       55

#define MODULE_RESPONSE_MAX     11
#define MODULE_RESPONSE_LEN     20

static const uint8_t MODULE_RESPONSE[MODULE_RESPONSE_MAX][MODULE_RESPONSE_LEN] = 
{
    "OK",
    "ERROR"
    "READY",
    "BUSY",
    "SLEEP",
    "Rx_MSG",
    "UNCON_DOWN",
    "JOIN_COMPLETE",
    "SEND",
    "RX1CH_OPEN",
    "RX2CH_OPEN",
};

typedef enum {
    OK = 0,
    __ERROR,
    READY,
    BUSY,
    SLEEP,
    RX_MSG,
    UNCON_DOWN,
    JOIN_COMPLETE,
    SEND,
    RX1CH_OPEN,
    RX2CH_OPEN,
} Wisol_ResposeTypeDef;

typedef enum {
    DEVRESET = 0x80,
    REPPERCHANGE = 0x81,
    REPIMMEDIATE = 0x82,
    EXTDEVMGMT = 0x00
} Rx_MSG_Control_TypeDef;

typedef struct {
    char rx_msg[6];
    char space;
    char fport;
    char ask;
    char version;
    char messagetype;
    char payloadlength;
    char payload[WISOL_BUFFER_SIZE];
} Rx_MSG_TypeDef;

typedef struct{
    char cmd[WISOL_BUFFER_SIZE];
    int  sizeofcmd;
    int  error;
    bool noreset;
    int  timeoutsec;
    Wisol_ResposeTypeDef respAck;
} Command_TypeDef;

static bool wisolInit = false;
static int  wisol_reboot = 0;
static callbackFunc callback = NULL;

static Command_TypeDef dataCmd;
static Command_TypeDef *currentCmd = NULL;

static TimerEvent_t responseTimer;

static void wakeup();
static void reset();
static void OnResponseEvent();
static void responseTimerOn(int timesec);
static void responseTimerOff();
static bool sendTx(const char *msg, int size, Wisol_ResposeTypeDef resptype);
static bool sendBinaryTx(const char *msg, int size, Wisol_ResposeTypeDef resptype);
static bool send(Command_TypeDef *sendmsg);

void initWisol(callbackFunc cb)
{
    if(callback == NULL)
    {
        callback = cb;
        wisol_reboot = 0;
        wisolInit = false;

        // To check Lora's port. 
        memset(&dataCmd, 0 , sizeof(Command_TypeDef));
        strcpy(dataCmd.cmd, "LRW 50 0\r\n");
        dataCmd.sizeofcmd = strlen(dataCmd.cmd);
        dataCmd.noreset = true;
        dataCmd.timeoutsec = WISOL_RESPONSE_TIME;
        dataCmd.respAck = OK; 

        currentCmd = &dataCmd;

        responseTimerOn(dataCmd.timeoutsec);
    }
}

bool writeLRW(const char *msg, int size, int bypasscmd)
{
    if(wisolInit) 
    {
        ///////////////////////////////////////////////////////////
        // sk iot spec
        if(bypasscmd == CID_TX_BYPASS_PROCESS_FOR_SKIOT) 
        {
            // sk specification for testing the deivce.
            if(callback != NULL && msg != NULL)
            {
                (*callback)(msg, size);
            }

        }
        ////////////////////////////////////////////////////////////

        if(size < WISOL_USER_DATA_LIMIT && currentCmd == NULL) 
        {
            return sendBinaryTx(msg, size, UNCON_DOWN);
        }
        else 
        {
            if(size >= WISOL_USER_DATA_LIMIT) 
            {
                DEBUG(ZONE_ERROR, ("The size of message is larger than the content that the LORA can transmit \r\n"));;    
            }
            else if(currentCmd != NULL)
            {
                DEBUG(ZONE_ERROR, ("In sending Message @@@@ \r\n"));;
            }
        }
    }
    else 
    {
        DEBUG(ZONE_ERROR, ("The lora is not ready !!! \r\n"));;
    }

    return false;
}

bool isInProcess()
{
    return (currentCmd != NULL)? true:false;
}

static void OnResponseEvent()
{
    DEBUG(ZONE_FUNCTION, ("+OnResponseEvent \r\n"));

    if(currentCmd != NULL)
    {
        if(currentCmd->timeoutsec == WISOL_RESPONSE_TIME)
        {
            if(currentCmd->noreset)
            {
                DEBUG(ZONE_FUNCTION, ("OnResponseEvent : no reset \r\n"));
                currentCmd->noreset = false;
                
                responseTimerOff();
                send(currentCmd);
            }
            else 
            {
                // if there is no reponse until limit itme, lora module may be dead.
                DEBUG(ZONE_ERROR, ("Lora module will need to start over @@@@ \r\n"));
                responseTimerOff();
                wisolInit = false;
                
                reset();
                currentCmd->noreset = true;

                // the command stored in buffer is retransmited after Lora has been reset.
                responseTimerOn(currentCmd->timeoutsec);
            }
        }
        else if(currentCmd->timeoutsec == WISOL_MAX_RESPONSE_TIME)
        {
            DEBUG(ZONE_FUNCTION, ("OnResponseEvent : Due to timeout, messages can no longer be delivered. \r\n"));

            currentCmd = NULL;

            responseTimerOff();
            LCMD_IOcontrol(LCOM_REPORT_TO_SERVER_FAILED, NULL, 0, NULL, 0);
        }
    }
    else 
    {
        responseTimerOff();
        DEBUG(ZONE_FUNCTION, ("OnResponseEvent : stop timer \r\n"));
    }
}

static void responseTimerOn(int timesec)
{
    DEBUG(ZONE_FUNCTION, ("responseTimerOn : timeron=%d \r\n", timesec));

    if(timesec == 0) 
    {
        return;    
    }

    TimerInit(&responseTimer, OnResponseEvent);
	TimerSetValue(&responseTimer, timesec);
	TimerStart(&responseTimer);
}

static void responseTimerOff()
{
    DEBUG(ZONE_FUNCTION, ("responseTimerOff \r\n", currentCmd));
	TimerStop(&responseTimer);
}


static bool sendTx(const char *msg, int size, Wisol_ResposeTypeDef resptype)
{
    bool ret = false;
    DEBUG(ZONE_FUNCTION, ("+sendTx : size=%d, resptype=%d \r\n"));    

    if(size > WISOL_BUFFER_SIZE - 12)
    {
        DEBUG(ZONE_ERROR, ("sendTx : The size of message is larger than the content\r\n"));;    
        return false;
    }

    memset(&dataCmd, 0 , sizeof(Command_TypeDef));
    char *cmd = dataCmd.cmd;
    
    *(cmd++) = 'L';
    *(cmd++) = 'R';
    *(cmd++) = 'W';
    *(cmd++) = ' ';
    *(cmd++) = '4';
    *(cmd++) = 'D';
    *(cmd++) = ' ';
    
    // Mtype
    if(resptype == UNCON_DOWN)
        *(cmd++) = 0;
    else
        *(cmd++) = 1;

    // Fport
    *(cmd++) = 1;
    
    // length
    *(cmd++) = size;

    // Message
    memcpy(cmd, msg, size);

    *(cmd+size) = '\r';      // CR
    *(cmd+size+1) = '\n';    // LF


    dataCmd.sizeofcmd = 12 + size;
    dataCmd.noreset = true;
    dataCmd.timeoutsec = WISOL_RESPONSE_TIME;
    dataCmd.respAck = resptype; 

    ret = send(&dataCmd);

    DEBUG(ZONE_FUNCTION, ("+sendTx : ret=%d \r\n", ret));;    

    return ret;


}

static bool sendBinaryTx(const char *msg, int size, Wisol_ResposeTypeDef resptype)
{
    bool ret = false;

    DEBUG(ZONE_FUNCTION, ("+sendBinaryTx : size=%d, resptype=%d\r\n"));

    if(size > WISOL_BUFFER_SIZE - 12)
    {
        DEBUG(ZONE_ERROR, ("sendBinaryTx : The size of message is larger than the content\r\n"));;    
        return false;
    }

    memset(&dataCmd, 0 , sizeof(Command_TypeDef));
    char *cmd = dataCmd.cmd;
    
    *(cmd++) = 'L';
    *(cmd++) = 'R';
    *(cmd++) = 'W';
    *(cmd++) = ' ';
    *(cmd++) = '4';
    *(cmd++) = 'D';
    *(cmd++) = ' ';
    
    // Mtype
    if(resptype == UNCON_DOWN)
        *(cmd++) = 0;
    else
        *(cmd++) = 1;

    // Fport
    *(cmd++) = 1;
    
    // length
    *(cmd++) = size;

    // Message
    memcpy(cmd, msg, size);

    *(cmd+size) = '\r';      // CR
    *(cmd+size+1) = '\n';    // LF


    dataCmd.sizeofcmd = 12 + size;
    dataCmd.noreset = false;
    dataCmd.timeoutsec = WISOL_RESPONSE_TIME;
    dataCmd.respAck = resptype; 


    ret = send(&dataCmd);

    
    DEBUG(ZONE_FUNCTION, ("+sendBinayTx : ret=%d \r\n", ret));;    

    return ret;
}

static bool send(Command_TypeDef *sendmsg)
{
    wakeup();

    if(callback != NULL && sendmsg != NULL)
    {
        currentCmd = sendmsg;
        (*callback)(currentCmd->cmd, currentCmd->sizeofcmd);
        responseTimerOn(currentCmd->timeoutsec);
        
        return true;
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

static bool responseProcess(const int index, const char *cmd, int size)
{
    // if it return true, the caller deletes the column in the database.
    bool ret = false;
    switch(index)
    {
    case OK:
        DEBUG(ZONE_FUNCTION , ("OK received!!!\r\n"));
        if(currentCmd != NULL && currentCmd->respAck == OK) 
        {
            DEBUG(ZONE_FUNCTION , ("OK received. command done\r\n"));
            responseTimerOff();
            currentCmd = NULL;
        }
        else 
        {
            if(currentCmd != NULL)
            {
                DEBUG(ZONE_FUNCTION , ("OK received. timeout is 50sec !!!\r\n"));

                responseTimerOff();

                currentCmd->timeoutsec = WISOL_MAX_RESPONSE_TIME;
                responseTimerOn(currentCmd->timeoutsec);
            }
            else
                DEBUG(ZONE_FUNCTION , ("OK received. There is no a waiting command \r\n"));
        }
        break;
    case __ERROR:
        DEBUG(ZONE_FUNCTION , ("ERROR received!!!\r\n"));
        if(currentCmd != NULL)
        {
            DEBUG(ZONE_FUNCTION , ("ERROR received. try it again\r\n"));
            responseTimerOff();
            
            currentCmd->timeoutsec = WISOL_RESPONSE_TIME;
            send(currentCmd);
        }
        break;
    case READY:
        DEBUG(ZONE_TRACE , ("READY received!!!\r\n"));
        if(currentCmd != NULL && currentCmd->respAck == READY) 
        {
            DEBUG(ZONE_FUNCTION , ("READY received and waiting for next command!!!\r\n"));
            currentCmd->respAck = OK;
            
            responseTimerOff();
            currentCmd = NULL;
            
            ret = true;
        }
        break;
    case BUSY:
        DEBUG(ZONE_TRACE , ("BUSY received!!!\r\n"));
        if(currentCmd != NULL)
        {
            DEBUG(ZONE_FUNCTION , ("BUSY received. timeout is 50sec !!!\r\n"));

            responseTimerOff();

            currentCmd->timeoutsec = WISOL_MAX_RESPONSE_TIME;
            responseTimerOn(currentCmd->timeoutsec);
        }
        else
            DEBUG(ZONE_FUNCTION , ("BUSY received. There is no a waiting command \r\n"));

        break;
    case SLEEP:
        DEBUG(ZONE_TRACE , ("SLEEP received!!!\r\n"));
        break;
    case RX_MSG:
        DEBUG(ZONE_TRACE , ("RX_MSG received!!!\r\n"));
        Rx_MSG_TypeDef rx_msg;
        memcpy(rx_msg, cmd, size);
        if(rx_msg.messagetype == EXTDEVMGMT)
        {
            if(rx_msg.payload[0] == 10)
            {
                int reporttime = rx_msg.payload[1];
                DEBUG(ZONE_WARN , ("Reporting cycle will change to %d\r\n", reporttime));
                DEMD_IOcontrol(DEMD_REPORT_PERIOD_CHANGE, &reporttime, 1, NULL, 0);
            }
        }
        ///////////////////////////////////////////////////////////
        // sk iot spec
        else if(rx_msg.messagetype == DEVRESET)
        {
            BSP_Delay_HW_Reset(); // time delay to allow lora module to be able to response Ack to N/W server 
        }
        break;
    case UNCON_DOWN:
        DEBUG(ZONE_TRACE , ("UNCON_DOWN received!!!\r\n"));
        if(currentCmd != NULL && currentCmd->respAck == UNCON_DOWN) 
        {
            DEBUG(ZONE_FUNCTION , ("UNCON_DOWN Received and waiting for ready!!!\r\n"));
            currentCmd->respAck = READY;
        }
        // a mission is succeed 
        break;
    case JOIN_COMPLETE:
        DEBUG(ZONE_TRACE , ("JOIN_COMPLETE received!!!\r\n"));
        wisolInit = true;
        break;

    case SEND:
         // the message is being sending. 
        if(currentCmd != NULL)
        {

        }
        else
        {
            //DEBUG(ZONE_ERROR , ("Impossible status @@@@ !!!\r\n"));
        }
        break;
    case RX1CH_OPEN:
    case RX2CH_OPEN:
        break;
    }

    return ret;
}

bool parser_Wisol(const char *cmd, int size)
{
    int i = 0;
    int cmdLength = 0;
    wisolInit = true;

    for(i = 0; i < MODULE_RESPONSE_MAX - 1  ; i++)
    {
        cmdLength = strlen((const char *)MODULE_RESPONSE[i]);
        if(!strncmp((const char*)cmd, (const char*)MODULE_RESPONSE[i], cmdLength))
        {
           return responseProcess(i, cmd, size);
        }
    }

    return false;
}