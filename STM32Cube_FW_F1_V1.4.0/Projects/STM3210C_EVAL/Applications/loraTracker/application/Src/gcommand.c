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
#include "stm3210c_loraTracker.h"
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
#define GPS_CMD_SIZE DATABASE_ELEMENT_DATA_SIZE

static char staticHold[] =  { 0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x00, 0x03,
                              0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 
                              0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x64, 0x3C, 
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                              0x00, 0x00, 0xB0, 0x94};

static char ana[] =         { 0xB5, 0x62, 0x06, 0x23, 0x28, 0x00, 0x00, 0x00, 0x4C, 0x66, 
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x16, 0x07, 0x00, 
                              0x00, 0x00, 0x00, 0x00, 0x9B, 0x06, 0x00, 0x00, 0x00, 0x00, 
                              0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0xE0 };

#if 0
static char cog[] =         {0xB5, 0x62, 0x06, 0x17, 0x0C, 0x00, 0x20, 0x23, 0x00, 0x02, 
                            0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x6F, 0x78};

static char ubx_g7020_kt[] = {0xB5, 0x62, 0x06, 0x23, 0x28, 0x00, 0x00, 0x00, 0x4C, 0x66, 
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x14, 0x06, 0x00, 
                            0x00, 0x00, 0x00, 0x00, 0xDC, 0x06, 0x00, 0x00, 0x00, 0x00, 
                            0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x02};                                                        

#endif

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

    GPRINTF(staticHold);
    GPRINTF(staticHold);
    GPRINTF(ana);
    GPRINTF(ana);
    //GPRINTF(cog);
    //GPRINTF(ubx_g7020_kt);
}

typedef enum {
    ON,
    OFF,
    TOGGLE
} Led_ActionType;

static void LedOnOffForGPS(Led_ActionType signal)
{
    static int count = 0;
    if(signal == TOGGLE)
    {
        count++;
        if(count < 256) 
        {
            BSP_LED_On(LED1);
        }
        else 
        {
            BSP_LED_Off(LED1);
            if(count > 512) {
                count = 0;
            }
        } 
    }
    else 
    {
        if(signal == ON) {
            BSP_LED_On(LED1);
        } else {
            BSP_LED_Off(LED1);
        }
    }
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

			// When gps data is valided, the led flashes.
            LedOnOffForGPS(TOGGLE);
        }
        else 
        {
			// When gps data is received, it turns on
            LedOnOffForGPS(ON);
        }
        if (command[i] == '\n')
        {
            if (i != 0 && i < GPS_CMD_SIZE)
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
                            if(updateDB(DEM, command, i+1, 0) != RQUEUE_OK)
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
            DEBUG(ZONE_ERROR, ("GCMD_Process : PARAM_OVERFLOW\r\n"));
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
    DEBUG(ZONE_FUNCTION, ("+GCMD_IOcontrol %d, 0x%x, %d, 0x%x, %d\r\n", io, input, insize, output, *outsize));

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

