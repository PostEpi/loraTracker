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
#include <math.h>
#include "stm32f1xx.h"
#include "nmea.h"
#include "gcommand.h"
#include "vdb.h"
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
#define GPS_CMD_SIZE 100

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
static void parse_cmd(const char *cmd);


static bool gcom_report_request = false;

/* Exported functions ---------------------------------------------------------*/

void GCMD_Init(void)
{
    gcom_Init();
    gcom_ReceiveInit();
}


    
void GCMD_Process(void)
{
    static char command[GPS_CMD_SIZE];
    static unsigned i = 0;
    static char *prmc = NULL, *pret = NULL;
    static char ch;

    /* Process all commands */
    while (gIsNewCharReceived() == SET)
    {
        ch = gGetNewChar();

#if 0 /* echo On    */
    DEBUG(ZONE_TRACE, ("%c", ch));
#endif
#if 1
        fusedata(ch);
        command[i] = ch;
        if(isdataready()) 
        {
            DEBUG(ZONE_GPS, ("%d:%d:%d %d-%d-%d\r\n", getHour(), getMinute(), getSecond(),  getYear(), getMonth(), getDay()))
            DEBUG(ZONE_GPS, ("latitude=%f, longitude=%f\r\n", getLatitude(), getLongitude()));
            DEBUG(ZONE_GPS, ("satelites=%d altitude=%d\r\n", getSatellites(), getAltitude())); 
            DEBUG(ZONE_GPS, ("speed=%d bearing=%d\r\n", getSpeed(), getBearing()));
        }
        if (command[i] == '\n')
        {
            if (i != 0)
            {
                if (updateDB(GPS, command, i+1, 0) != RQUEUE_OK)
                {
                    DEBUG(ZONE_ERROR, ("GCMD_Process : Update is failed to GPS\r\n"));
                }

                prmc = strstr((const char *)command, (const char *)"$GPRMC");
                if (prmc != NULL)
                {
                    pret = NULL;
                    pret = strrchr((const char*)prmc, '\n');
                    if(pret)
                    {
                        // if there is a request for periodic reporting, nmea have to be stored in the database of DEM.
                        if(gcom_report_request) 
                        {
                            gcom_report_request = false;
                            if(updateDB(DEM, command, i, 0) != RQUEUE_OK)
                            {
                                DEBUG(ZONE_ERROR, ("GCMD_Process : Update is failed to DEM\r\n"));
                            }
                        }

                        DEBUG(ZONE_TRACE, ("GCMD_Process %s", command));
                    }
                }
                i = 0;
                memset((void*)command, 0, GPS_CMD_SIZE);
                return;
            }
        }
        else if (i > (GPS_CMD_SIZE - 1))
        {
            i = 0;
            memset((void*)command, 0, GPS_CMD_SIZE);
            //com_error(AT_TEST_PARAM_OVERFLOW);
            DEBUG(ZONE_ERROR, ("GPS PARAM_OVERFLOW"));
        }
        else
        {
            i++;
        }
#endif        
    }
}


COM_StatusTypeDef GCMD_IOcontrol(GCOM_IOControlTypedef io, int *input, int insize, int *output, int *outsize) 
{
    COM_StatusTypeDef ret = COM_OK;
    DEBUG(ZONE_FUNCTION, ("+GCMD_IOcontrol %d, 0x%x, %d, 0x%x, %d\r\n", io, input, insize, output, outsize));

    switch(io)
    {
        case GCOM_REPORT_REQUEST:
            gcom_report_request = true;
            break;
        default:
            ret = COM_PARAM_ERROR;
    }

    DEBUG(ZONE_FUNCTION, ("-GCMD_IOcontrol %d\r\n", ret));
    return ret;
}

/* Private functions ---------------------------------------------------------*/

