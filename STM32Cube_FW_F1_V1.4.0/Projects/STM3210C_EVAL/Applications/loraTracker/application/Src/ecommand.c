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
#include <string.h>
#include "stm32f1xx.h"
//#include "at.h"
#include "ecommand.h"
#include "vdb.h"
#include "bbox.h"
#include "debug.h"


/* comment the following to have help message */
/* #define NO_HELP */
/* #define NO_KEY_ADDR_EUI */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CMD_SIZE 128

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  Print a string corresponding to an ATEerror_t
 * @param  The AT error code
 * @retval None
 */
//static void com_error(ATEerror_t error_type);

/**
 * @brief  Parse a command and process it
 * @param  The command
 * @retval None
 */
static bool parse_cmd(const char *cmd, int size);

/* Exported functions ---------------------------------------------------------*/

void ECMD_Init(void)
{
    ecom_Init();
    ecom_ReceiveInit();
}


void ECMD_Process(void)
{
    static char command[CMD_SIZE];
    static unsigned i = 0;

    element item;
    DB_TypeDef db = GPS;

    /* Process all commands */
    while (eIsNewCharReceived() == SET)
    {
        command[i] = eGetNewChar();

#if 1 /* echo On    */
        DEBUG(ZONE_TRACE, ("%c", command[i]));
#endif

        if ((command[i] == '\r') || (command[i] == '\n'))
        {
            if (i != 0)
            {

                if(i < BBOX_MESSAGE_SIZE) 
                {
                    if(parse_cmd(command, i))
                    {
                        EPRINTF("ok\r\n");
                        
                        if (updateDB(DEM, command, i+1, 0) != RQUEUE_OK)
                        {
                            DEBUG(ZONE_ERROR, ("ECMD_Process : Update is failed to DEM @@@@\r\n"));
                        }
                    }
                }
                i = 0;
                memset((void*)command, 0, CMD_SIZE);
            }
        }
        else if (i == (CMD_SIZE - 1))
        {
            DEBUG(ZONE_TRACE, ("ECMD_Process : command bffer overflow = %s\r\n", command));

            i = 0;
            memset((void*)command, 0, CMD_SIZE);
        }
        else 
        {
            i++;
        }
    }
    
    while (!isEmptydDB(db) && selectDB(db, &item) == RQUEUE_OK)
    {
        DEBUG(ZONE_TRACE, ("ECMD_Process : %d, %d, %s\r\n", item.size, item.retcount, item.edata ))
        EPRINTF(item.edata);
        deleteDB(db, &item);
    }
}

/* Private functions ---------------------------------------------------------*/
static bool parse_cmd(const char *cmd, int size)
{
    bool ret = false;
    bool bCmdValided = false;
    bool bReset = false;

    if(strstr((const char *)cmd, (const char *)BBOX_STX_BERDP) != NULL)
    {
        bCmdValided = true;
    }
    else if(strstr((const char *)cmd, (const char *)BBOX_STR_BURDP) != NULL)
    {
        bCmdValided = true;
    }
    else if(strstr((const char *)cmd, (const char *)BBOX_STX_BDRDP) != NULL)
    {
        bCmdValided = true;
        bReset = true;
    }

    if(bCmdValided)
    {
        parsebbox(cmd, size);
        if(isbboxready())
        {
            DEBUG(ZONE_FUNCTION, ("parse_cmd : %s\r\n", cmd));
            if(bReset) 
            {
                DEBUG(ZONE_FUNCTION, ("   *** Let's go to the downloading process ***\r\n"));
    
                // system reset test
                //BSP_Download_Reset();
            }
            ret = true;
        }
        else 
        {
            DEBUG(ZONE_ERROR, ("parse_cmd : Invalid value @@@@ %s\r\n", cmd));
        }
    }

    return ret;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
