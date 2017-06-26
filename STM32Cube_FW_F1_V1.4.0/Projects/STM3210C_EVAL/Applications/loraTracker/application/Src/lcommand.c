/*******************************************************************************
 * @file    command.c
 * @author  MCD Application Team
 * @version V1.1.0
 * @date    27-February-2017
 * @brief   main command driver dedicated to command AT
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
 * All rights reserved.</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "stm32f1xx.h"
//#include "at.h"
#include "lcommand.h"
#include "vdb.h"
#include "demd.h"
#include "wisol.h"
#include "debug.h"

/* comment the following to have help message */
/* #define NO_HELP */
/* #define NO_KEY_ADDR_EUI */

/* Private typedef -----------------------------------------------------------*/
/**
 * @brief  Structure defining an AT Command
 */
#if 0
struct ATCommand_s {
  const char *string;                       /*< command string, after the "AT" */
  const int size_string;                    /*< size of the command string, not including the final \0 */
  ATEerror_t (*get)(const char *param);     /*< =? after the string to get the current value*/
  ATEerror_t (*set)(const char *param);     /*< = (but not =?\0) after the string to set a value */
  ATEerror_t (*run)(const char *param);     /*< \0 after the string - run the command */
#if !defined(NO_HELP)
  const char *help_string;                  /*< to be printed when ? after the string */
#endif
};
#endif

/* Private define ------------------------------------------------------------*/
#define CMD_SIZE 128

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 * @brief  Array corresponding to the description of each possible AT Error
 */
static const char *const ATError_description[] =
    {
        "\r\nOK\r\n",                     /* AT_OK */
        "\r\nAT_ERROR\r\n",               /* AT_ERROR */
        "\r\nAT_PARAM_ERROR\r\n",         /* AT_PARAM_ERROR */
        "\r\nAT_BUSY_ERROR\r\n",          /* AT_BUSY_ERROR */
        "\r\nAT_TEST_PARAM_OVERFLOW\r\n", /* AT_TEST_PARAM_OVERFLOW */
        "\r\nAT_NO_NETWORK_JOINED\r\n",   /* AT_NO_NET_JOINED */
        "\r\nAT_RX_ERROR\r\n",            /* AT_RX_ERROR */
        "\r\nerror unknown\r\n",          /* AT_MAX */
};

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  Print a string corresponding to an ATEerror_t
 * @param  The AT error code
 * @retval None
 */
/**
 * @brief  Parse a command and process it
 * @param  The command
 * @retval None
 */
static void writeCom(const char *cmd, int size);
static void parse_cmd(const char *cmd);

/* Exported functions ---------------------------------------------------------*/

void LCMD_Init(void)
{
    lcom_Init();
    lcom_ReceiveInit();

    initWisol(writeCom);
}

void LCMD_Process(void)
{
    static char command[CMD_SIZE];
    static unsigned i = 0;

    element item;
    DB_TypeDef db = LOR;

    /* Process all commands */
    while (lIsNewCharReceived() == SET)
    {
        command[i] = lGetNewChar();
        
#if 0 /* echo On    */
        DEBUG(ZONE_TRACE, ("%c", command[i]));
#endif  
        if (/*(command[i] == '\r') || */(command[i] == '\n'))
        {
            if (i != 0)
            {
                if(parser_Wisol(command, i+1))
                {   
                    // data has been sent to server through Lora network.
                    DEMD_IOcontrol(DEMD_REPORT_TO_SERVER_SUCCEED, NULL, 0, NULL, 0);
                    deleteDB(db, &item);
                }

                DEBUG(ZONE_FUNCTION, ("LCMD_Process : %s", command));
                i = 0;
                memset((void*)command, 0, CMD_SIZE);
            }
        }
        else if (i == (CMD_SIZE - 1))
        {
            
            DEBUG(ZONE_ERROR, ("LORA PARAM_OVERFLOW"));;
            i = 0;
            memset((void*)command, 0, CMD_SIZE);
        }
        else
        {
            i++;
        }
    }
    
    if (!isEmptydDB(db) && selectDB(db, &item) == RQUEUE_OK)
    {
        DEBUG(ZONE_FUNCTION, ("LCMD_Process : %d, %d, %s\r\n", item.size, item.retcount, item.edata ))
        //LPRINTF("LRW30otaa\r\n");
        writeLRW(item.edata, item.size);
    }
}


COM_StatusTypeDef LCMD_IOcontrol(LCOM_IOControlTypedef io, int *input, int insize, int *output, int *outsize) 
{
    COM_StatusTypeDef ret = COM_OK;
    DEBUG(ZONE_FUNCTION, ("+LCMD_IOcontrol %d, 0x%x, %d, 0x%x, %d\r\n", io, input, insize, output, outsize));

    switch(io)
    {
        case LCOM_REPORT_TO_SERVER_FAILED:
            element item;
            
            DEBUG(ZONE_WARN, ("LCMD_IOcontrol : Sending message is failed. So database item has to be deleted \r\n"));
            DEMD_IOcontrol(DEMD_REPORT_TO_SERVER_FAILED, NULL, 0, NULL, 0);
            deleteDB(LOR, &item);
            break;
        default:
            ret = COM_PARAM_ERROR;
    }

    DEBUG(ZONE_FUNCTION, ("-LCMD_IOcontrol %d\r\n", ret));
    return ret;
}

/* Private functions ---------------------------------------------------------*/
static void writeCom(const char *cmd, int size)
{
    DEBUG(ZONE_FUNCTION, ("writeCom : %d, %s\r\n", size, cmd ));
    LBPRINTF(cmd, size);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
