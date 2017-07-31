#include <stdlib.h>
#include <string.h>
#include "stm32f1xx.h"
#include "wisol.h"
#include "time.h"
#include "timtimer.h"
#include "timeServer.h"
#include "demd.h"
#include "lcommand.h"
#include "debug.h"

#define WISOL_MAX_RESPONSE_TIME 50000   
#define WISOL_RESPONSE_TIME     10000

#define MODULE_RESPONSE_MAX 20
#define MODULE_RESPONSE_LEN 35

static const uint8_t MODULE_RESPONSE[MODULE_RESPONSE_MAX][MODULE_RESPONSE_LEN] =
    {
        "OK",
        "ERROR",
        "READY",
        "BUSY",
        "SLEEP",
        "RX_MSG_FP222_LEN",
        "CON_DOWN",
        "UNCON_DOWN",
        "JOIN_COMPLETE",
        "FRAME_TYPE_JOIN_ACCEPT",
        "SEND",
        "RX1CH_OPEN",
        "RX2CH_OPEN",
        "DEVRESET",
        "INTERVAL",
        "EXTDEVMGMT",
        "Rx_MSG",
        "FRAME_TYPE_DATA_CONFIRMED_DOWN",
        "FRAME_TYPE_DATA_UNCONFIRMED_DOWN",
        "WAKE_UP",
};

typedef enum {
    OK = 0,
    __ERROR,
    READY,
    BUSY,
    SLEEP,
    RX_MSG_FP222_LEN,
    CON_DOWN,
    UNCON_DOWN,
    JOIN_COMPLETE,
    FRAME_TYPE_JOIN_ACCEPT,
    SEND,
    RX1CH_OPEN,
    RX2CH_OPEN,
    DEVRESET,
    INTERVAL,
    EXTDEVMGMT,
    Rx_MSG,
    FRAME_TYPE_DATA_CONFIRMED_DOWN,
    FRAME_TYPE_DATA_UNCONFIRMED_DOWN,
    WAKE_UP,
} Wisol_ResposeTypeDef;

typedef enum {
    RX_MSG_CMD_DEVRESET = 0x80,
    RX_MSG_CMD_REPPERCHANGE = 0x81,
    RX_MSG_CMD_REPIMMEDIATE = 0x82,
    RX_MSG_CMD_EXTDEVMGMT = 0x00
} RX_MSG_CMD_Control_TypeDef;

typedef struct
{
    char rx_msg[6];
    char space1;
    char fport[3];
    char space2;
    char ask;
    char space3;
    char version[2];
    char messagetype[2];
    char payloadlength[2];
    char payload[WISOL_USER_DATA_SIZE];
} Rx_MSG_TypeDef;

typedef struct
{
    char cmd[WISOL_LRW_CMD_DATA_SIZE];
    int sizeofcmd;
    int error;
    bool noreset;
    int timeoutsec;
    Wisol_ResposeTypeDef respAck;
} LRW_Command_TypeDef;

static bool wisolInit = false;
static int transferFail = 0;
static callbackFunc callback = NULL;

static LRW_Command_TypeDef dataCmd;
static LRW_Command_TypeDef *currentCmd = NULL;

static TimerEvent_t responseTimer;
static int factoryTest = 0;

#ifdef DEMD_IOT_SK_TEST_SPEC
static int sendSkiot = 0;
static int successSkiot = 0; 
#endif

static void wakeup();
static void reset();
static void OnResponseEvent();
static void responseTimerOn(int timesec);
static void responseTimerOff();
static bool sendTx(const char *msg, int size, Wisol_ResposeTypeDef resptype);
static bool sendBinaryTx(const char *msg, int size, Wisol_ResposeTypeDef resptype);
static bool send(LRW_Command_TypeDef *sendmsg);

void initWisol(callbackFunc cb)
{
    if (callback == NULL)
    {
        callback = cb;
        wisolInit = false;
#ifdef DEMD_IOT_SK_TEST_SPEC
        reset();
#else        
        factoryTest = 1; //BSP_Input_GetState(INPUT_FACTORY);
#endif
        BSP_LED_On(LED2);

        // To check Lora's port and debug mode is off.
        memset(&dataCmd, 0, sizeof(LRW_Command_TypeDef));
#ifdef DEMD_IOT_SK_TEST_SPEC     
        strcpy(dataCmd.cmd, "LRW 50 1\r\n");
#else
        strcpy(dataCmd.cmd, "LRW 50 0\r\n");
#endif 
        dataCmd.sizeofcmd = strlen(dataCmd.cmd);
        dataCmd.noreset = true;
        dataCmd.timeoutsec = 2000;//WISOL_RESPONSE_TIME;
        dataCmd.respAck = OK;

        // The first command is executed after a few seconds because Lora needs a intialization time.
        currentCmd = &dataCmd;
        responseTimerOn(dataCmd.timeoutsec);
       
    }
}

bool writeLRW(const char *msg, int size, int bypasscmd)
{
    if (wisolInit)
    {
        ///////////////////////////////////////////////////////////
        // sk iot spec
        if (bypasscmd == CID_TX_BYPASS_PROCESS_FOR_SKIOT)
        {
            // sk specification for testing the deivce.
            if (callback != NULL && msg != NULL)
            {
                wakeup();

                (*callback)(msg, size);
                LCMD_IOcontrol(LCOM_REPORT_TO_SERVER_FAILED, NULL, 0, NULL, 0);
            }
        }
        ////////////////////////////////////////////////////////////

        if (size <= WISOL_USER_DATA_LIMIT && currentCmd == NULL)
        {
#ifdef DEMD_IOT_SK_TEST_SPEC
            sendSkiot++;
            //char *skmsg = "01234567890123456789012345678901234567890123456789012345678901234";
            char *skmsg = "0";
            size = strlen(skmsg);
            return sendBinaryTx(skmsg, size, OK); 
#else           
            return sendBinaryTx(msg, size, UNCON_DOWN);
#endif            
        }
        else
        {
            if (size > WISOL_USER_DATA_LIMIT)
            {
                DEBUG(ZONE_ERROR, ("The size of message is larger than the content that the LORA can transmit \r\n"));
                ;
            }
            else if (currentCmd != NULL)
            {
                DEBUG(ZONE_ERROR, ("In sending Message @@@@ \r\n"));
            }
        }
    }
    else
    {
        DEBUG(ZONE_ERROR, ("Lora is not ready !!! \r\n"));
        ;
    }

    return false;
}

bool isTxReady()
{
    return wisolInit;
}

bool isInProcess()
{
    return (currentCmd != NULL) ? true : false;
}

static void OnResponseEvent()
{
    DEBUG(ZONE_FUNCTION, ("+OnResponseEvent \r\n"));

    if (currentCmd != NULL)
    {
        if (currentCmd->timeoutsec <= WISOL_RESPONSE_TIME)
        {
            if (currentCmd->noreset)
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
        else if (currentCmd->timeoutsec == WISOL_MAX_RESPONSE_TIME)
        {
            transferFail++;
            currentCmd = NULL;

            DEBUG(ZONE_FUNCTION, ("OnResponseEvent : Due to timeout, messages can no longer be delivered. %d \r\n", transferFail));

			// Indicates the transmission failure with LED2
            BSP_LED_On(LED2);
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

    if (timesec == 0)
    {
        return;
    }

    TimerInit(&responseTimer, OnResponseEvent);
    TimerSetValue(&responseTimer, timesec);
    TimerStart(&responseTimer);
}

static void responseTimerOff()
{
    DEBUG(ZONE_FUNCTION, ("responseTimerOff \r\n"));
    TimerStop(&responseTimer);
}

static bool sendTx(const char *msg, int size, Wisol_ResposeTypeDef resptype)
{
    bool ret = false;
    DEBUG(ZONE_FUNCTION, ("+sendTx : size=%d, resptype=%d \r\n", size, resptype));

    if (size > WISOL_USER_DATA_LIMIT)
    {
        DEBUG(ZONE_ERROR, ("sendTx : The size of message is larger than the content\r\n"));
        ;
        return false;
    }

    memset(&dataCmd, 0, sizeof(LRW_Command_TypeDef));
    char *cmd = dataCmd.cmd;

    *(cmd++) = 'L';
    *(cmd++) = 'R';
    *(cmd++) = 'W';
    *(cmd++) = ' ';
    *(cmd++) = '4';
    *(cmd++) = 'D';
    *(cmd++) = ' ';

    // Mtype
    if (resptype == UNCON_DOWN)
        *(cmd++) = 0;
    else
        *(cmd++) = 1;

    // Fport
    *(cmd++) = 1;

    // length
    *(cmd++) = size;

    // Message
    memcpy(cmd, msg, size);

    *(cmd + size) = '\r';     // CR
    *(cmd + size + 1) = '\n'; // LF

    dataCmd.sizeofcmd = 12 + size;
    dataCmd.noreset = true;
    dataCmd.timeoutsec = WISOL_RESPONSE_TIME;
    dataCmd.respAck = resptype;

    ret = send(&dataCmd);

    DEBUG(ZONE_FUNCTION, ("+sendTx : ret=%d \r\n", ret));
    ;

    return ret;
}

static bool sendBinaryTx(const char *msg, int size, Wisol_ResposeTypeDef resptype)
{
    bool ret = false;

    DEBUG(ZONE_FUNCTION, ("+sendBinaryTx : size=%d, resptype=%d\r\n", size, resptype));

    if (size > WISOL_USER_DATA_LIMIT)
    {
        DEBUG(ZONE_ERROR, ("sendBinaryTx : The size of message is larger than the content\r\n"));
        ;
        return false;
    }

    memset(&dataCmd, 0, sizeof(LRW_Command_TypeDef));
    char *cmd = dataCmd.cmd;

    *(cmd++) = 'L';
    *(cmd++) = 'R';
    *(cmd++) = 'W';
    *(cmd++) = ' ';
    *(cmd++) = '4';
    *(cmd++) = 'D';
    *(cmd++) = ' ';

    // Mtype
    if (resptype == UNCON_DOWN)
        *(cmd++) = 1;
    else
        *(cmd++) = 0;

    // Fport
    *(cmd++) = 1;

    // length
    *(cmd++) = size;
    memcpy(cmd, msg, size);
   

    *(cmd + size) = '\r';     // CR
    *(cmd + size + 1) = '\n'; // LF

    dataCmd.sizeofcmd = 12 + size;
    dataCmd.noreset = false;
    dataCmd.timeoutsec = WISOL_RESPONSE_TIME;
    dataCmd.respAck = resptype;

    ret = send(&dataCmd);

    DEBUG(ZONE_FUNCTION, ("+sendBinayTx : ret=%d \r\n", ret));
    ;

    return ret;
}

static bool send(LRW_Command_TypeDef *sendmsg)
{
    wakeup();

    if (callback != NULL && sendmsg != NULL)
    {
        BSP_LED_On(LED2);

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

#ifndef DEMD_IOT_SK_TEST_SPEC
static void getRssi()
{
    strcpy(dataCmd.cmd, "LRW 4A\r\n");
    dataCmd.sizeofcmd = strlen(dataCmd.cmd);
    dataCmd.noreset = true;
    dataCmd.timeoutsec = 2000;//WISOL_RESPONSE_TIME;
    dataCmd.respAck = OK;

    // The first command is executed after a few seconds because Lora needs a intialization time.
    currentCmd = &dataCmd;
    responseTimerOn(dataCmd.timeoutsec);
}
#endif

static bool externalDevManage(const char *cmd, int sizeofextdev)
{
    if(cmd != NULL && sizeofextdev > 1)
    {
        if (cmd[0] == '1' && cmd[1] == '0')
        {
            int itime;
            char ctime[2];
            ctime[0] = cmd[2];
            ctime[1] = cmd[3];

            if(ISVALIDHEX(ctime[0]) && ISVALIDHEX(ctime[1]))
            {
                itime = digit2dec(ctime[0]) * 16 + digit2dec(ctime[1]);
                DEBUG(ZONE_FUNCTION, ("The report cycle will be changed to %d.\r\n", itime));
                DEMD_IOcontrol(DEMD_REPORT_PERIOD_CHANGE, &itime, 1, NULL, 0);
                
                return true;
            }
            else 
            {
                DEBUG(ZONE_ERROR, ("The value of report cycle is invalid @@@@.\r\n"));
            }
        }
        else 
        {
            DEBUG(ZONE_ERROR, ("The report cycle is invalid @@@@.\r\n"));
        }
    }
    
    return false;
}

static int rxmsgpf222Parser(int index, const char *cmd, int size, char *outmsg, int outsize)
{
    int count = strlen((const char *)MODULE_RESPONSE[index]);
    int datacount = size - count;
    bool bfound = false;
    char number[4];
    char ch;
    int sudochar = 0;
    int msgCount = 0;

    DEBUG(ZONE_FUNCTION, ("rxmsgpf222Parser !!!\r\n"));

    cmd += count; // Remove header part and move to acual data location. 
    count = 0;
    memset(outmsg, 0, outsize);
    memset(number, 0, 4);
    while (datacount-- > 0)
    {
        ch = *(cmd++);
        if (bfound && count < outsize )
            outmsg[count++] = ch;
        else
        {
            if (ch == ' ' || ch == ':')
            {
                sudochar++;
                if (sudochar >= 3)
                {
                    bfound = true;
                    count = 0;
                    msgCount = atoi(number);
                }
            }
            else
            {
                number[count++] = ch;
            }
        }
    }

    // if(msgCount != count) 
    // {
    //      DEBUG(ZONE_ERROR, ("Error : The size of received data is different. @@@@ %d, %d!!!\r\n", msgCount, count ));
    // }

    return bfound? msgCount : 0;
}

static void rxmsgparser(const char *cmd, int size)
{
    int which, payloadlength;
    Rx_MSG_TypeDef rxtype;
    memset((void *)&rxtype, 0, sizeof(Rx_MSG_TypeDef));
    memcpy((void *)&rxtype, cmd, size);
    //which = atoi(rxtype.messagetype);
    which = digit2dec(rxtype.messagetype[0]) * 16 + digit2dec(rxtype.messagetype[1]);
    switch (which)
    {
    case RX_MSG_CMD_DEVRESET:
        DEBUG(ZONE_FUNCTION, ("System reset after 1sec..\r\n"));
        BSP_Delay_HW_Reset(); // time delay to allow lora module to be able to response Ack to N/W server
        break;
    case RX_MSG_CMD_REPPERCHANGE:
        payloadlength = digit2dec(rxtype.payloadlength[0]) * 16 + digit2dec(rxtype.payloadlength[1]);
        DEBUG(ZONE_FUNCTION, ("rxmsgparser : RX_MSG_CMD_REPPERCHANGE payloadlength=%d.\r\n", payloadlength));
        if (payloadlength)
        {
            int delaytime = digit2dec(rxtype.payload[0]) * 16 + digit2dec(rxtype.payload[1]);
            DEBUG(ZONE_FUNCTION, ("Interval will be changed to %d.\r\n", delaytime));
        }
        break;
    case RX_MSG_CMD_REPIMMEDIATE:
        DEBUG(ZONE_FUNCTION, ("System reset after 1sec..\r\n"));
        break;
    case RX_MSG_CMD_EXTDEVMGMT:
        payloadlength = digit2dec(rxtype.payloadlength[0]) * 16 + digit2dec(rxtype.payloadlength[1]);
        DEBUG(ZONE_FUNCTION, ("rxmsgparser : RX_MSG_CMD_EXTDEVMGMT payloadlength=%d.\r\n", payloadlength));
        if (payloadlength)
        {
            externalDevManage(rxtype.payload, payloadlength);
        }
        break;
    }
}


#define RX_MSG_BUFFER 130

static bool responseProcess(const int index, const char *cmd, int size)
{
// if it return true, the caller deletes the column in the database.
#ifdef DEMD_IOT_SK_TEST_SPEC
    static bool skiotpec = false;
#endif
    static char rxmsg[RX_MSG_BUFFER];
    static int rxmsgcnt = 0;
    bool ret = false;
    switch (index)
    {
    case OK:
        DEBUG(ZONE_TRACE, ("OK received!!!\r\n"));
        if (currentCmd != NULL && currentCmd->respAck == OK)
        {
 
             // When the transmission is completed, it is led off.
            BSP_LED_Off(LED2);

            responseTimerOff();
            currentCmd = NULL;

#ifdef DEMD_IOT_SK_TEST_SPEC
            DEBUG(ZONE_FUNCTION, ("OK received. try=%d, success=%d\r\n", sendSkiot, successSkiot));

            successSkiot++;
            ret = true;
#endif
        }
        else
        {
            if (currentCmd != NULL)
            {
                DEBUG(ZONE_FUNCTION, ("OK received. timeout is 50sec !!!\r\n"));

                currentCmd->respAck = UNCON_DOWN;
                responseTimerOff();

                currentCmd->timeoutsec = WISOL_MAX_RESPONSE_TIME;
                responseTimerOn(currentCmd->timeoutsec);
            }
            else
                DEBUG(ZONE_FUNCTION, ("OK received. There is no a waiting command \r\n"));
        }
        break;
    case __ERROR:
        DEBUG(ZONE_FUNCTION, ("ERROR received!!!\r\n"));
        if (currentCmd != NULL)
        {
            DEBUG(ZONE_FUNCTION, ("ERROR received. try it again\r\n"));
            responseTimerOff();

            currentCmd->timeoutsec = WISOL_RESPONSE_TIME;
            send(currentCmd);
        }
        break;
    case READY:
        DEBUG(ZONE_TRACE, ("READY received!!!\r\n"));

        if (currentCmd != NULL && currentCmd->respAck == READY)
        {
            DEBUG(ZONE_FUNCTION, ("READY received and waiting for next command!!!\r\n"));
        }
        break;
    case BUSY:
        DEBUG(ZONE_TRACE, ("BUSY received!!!\r\n"));
#ifdef DEMD_IOT_SK_TEST_SPEC
        if(currentCmd != NULL)
        {
            BSP_LED_Off(LED2);

            responseTimerOff();
            currentCmd = NULL;
        }
#endif
#if 0
        if(currentCmd != NULL)
        {
            DEBUG(ZONE_FUNCTION , ("BUSY received. timeout is 50sec !!!\r\n"));

            responseTimerOff();

            currentCmd->timeoutsec = WISOL_MAX_RESPONSE_TIME;
            responseTimerOn(currentCmd->timeoutsec);
        }
        else
            DEBUG(ZONE_FUNCTION , ("BUSY received. There is no a waiting command \r\n"));
#endif
        break;
    case SLEEP:
        DEBUG(ZONE_TRACE, ("SLEEP received!!!\r\n"));
        break;
    case RX_MSG_FP222_LEN:
        rxmsgcnt = 0;
        rxmsgcnt = rxmsgpf222Parser(index, cmd, size, rxmsg, RX_MSG_BUFFER);
        break;
    case Rx_MSG:
        rxmsgparser(cmd, size);
        break;
    case FRAME_TYPE_DATA_CONFIRMED_DOWN:
    case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
    case CON_DOWN:
    case UNCON_DOWN:
        DEBUG(ZONE_FUNCTION, ("%s received!!!\r\n", MODULE_RESPONSE[index]));
        if (currentCmd != NULL && currentCmd->respAck == UNCON_DOWN)
        {
            DEBUG(ZONE_FUNCTION, ("Sending data is completed !!!\r\n"));
            responseTimerOff();
            currentCmd = NULL;

            // When the transmission is completed, it is led off.
            BSP_LED_Off(LED2);

            ret = true;

#ifndef DEMD_IOT_SK_TEST_SPEC
            if (factoryTest)
            {
                getRssi();
            }
#endif

        }
        // a mission is succeed
        break;
    case JOIN_COMPLETE:
    case FRAME_TYPE_JOIN_ACCEPT:
        DEBUG(ZONE_FUNCTION, ("JOIN_COMPLETE received!!!\r\n"));
        responseTimerOff();
        currentCmd = NULL;
        wisolInit = true;
#ifdef DEMD_IOT_SK_TEST_SPEC
        if(!skiotpec) 
        {
            skiotpec = true;
            wisolInit = false;
            reset();
        }
#endif


        break;

    case SEND:
        // the message is being sending.
        if (currentCmd != NULL)
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

    case DEVRESET:
        DEBUG(ZONE_FUNCTION, ("System reset after 1sec..\r\n"));
        BSP_Delay_HW_Reset(); // time delay to allow lora module to be able to response Ack to N/W server
        break;
    case INTERVAL:
        if (rxmsgcnt)
        {
            DEBUG(ZONE_FUNCTION, ("interval will be changed to %d.\r\n", digit2dec(rxmsg[0]) * 16 + digit2dec(rxmsg[1])));
        }
        break;
    case EXTDEVMGMT:
        DEBUG(ZONE_FUNCTION, ("EXTDEVMGMT : %d, %s\r\n", rxmsgcnt, rxmsg));
        externalDevManage(rxmsg, rxmsgcnt);
        break;
    case WAKE_UP:
		// When Lora wakes up, it turns on.
        //BSP_LED_On(LED2);
        break;
    }

    return ret;
}

static void upper_string(char s[])
{
    int c = 0;

    while (s[c] != '\0')
    {
        if (s[c] >= 'a' && s[c] <= 'z')
        {
            s[c] = s[c] - 32;
        }
        c++;
    }
}

bool parser_Wisol(char *cmd, int size)
{
    int i = 0;
    int cmdLength = 0;
    wisolInit = true;

    //upper_string(cmd);

    for (i = 0; i < MODULE_RESPONSE_MAX; i++)
    {
        cmdLength = strlen((const char *)MODULE_RESPONSE[i]);
        //DEBUG(ZONE_FUNCTION, ("%s, %s, %d\r\n", cmd, MODULE_RESPONSE[i], cmdLength));
        //if (!strncmp((const char *)cmd, (const char *)MODULE_RESPONSE[i], cmdLength))
        if (strstr((const char *)cmd, (const char *)MODULE_RESPONSE[i]) != NULL)
        {
            //DEBUG(ZONE_FUNCTION, ("find %s, %d\r\n", MODULE_RESPONSE[i], i));
            return responseProcess(i, cmd, size);
        }
    }

    return false;
}