/**
  ******************************************************************************
  * @file    stm3210c_eval.c
  * @author  MCD Application Team
  * @version V6.0.2
  * @date    29-April-2016
  * @brief   This file provides a set of firmware functions to manage Leds, 
  *          push-button and COM ports for STM3210C_EVAL
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm3210c_loraTracker.h"
#include "flash_if.h"

/** @addtogroup BSP
  * @{
  */

/** @defgroup STM3210C_EVAL STM3210C-EVAL
  * @{
  */

/** @defgroup STM3210C_EVAL_COMMON STM3210C-EVAL Common
  * @{
  */

/** @defgroup STM3210C_EVAL_Private_TypesDefinitions Private Types Definitions
  * @{
  */

typedef struct
{
    __IO uint16_t LCD_REG_R; /* Read Register */
    __IO uint16_t LCD_RAM_R; /* Read RAM */
    __IO uint16_t LCD_REG_W; /* Write Register */
    __IO uint16_t LCD_RAM_W; /* Write RAM */
} TFT_LCD_TypeDef;

/**
  * @}
  */

/** @defgroup STM3210C_EVAL_Private_Defines Private Defines
  * @{
  */

/* LINK LCD */
#define START_BYTE 0x70
#define SET_INDEX 0x00
#define READ_STATUS 0x01
#define LCD_WRITE_REG 0x02
#define LCD_READ_REG 0x03

/* LINK SD Card */
#define SD_DUMMY_BYTE 0xFF
#define SD_NO_RESPONSE_EXPECTED 0x80

/* Iot Tracker version number */
#define __BOOTLOADER_VERSION        (1)
#define __APPLICATION_VERSION       (3)

#if 0
__APPLICATION_VERSION: application version history.
1 : base source
2 : the codes for sk iot is added.
3 : gps tunning
#endif


/**
 * @brief STM3210C EVAL BSP Driver version number
   */
#define __STM3210C_EVAL_BSP_VERSION_MAIN (0x06) /*!< [31:24] main version */
#define __STM3210C_EVAL_BSP_VERSION_SUB1 (0x00) /*!< [23:16] sub1 version */
#define __STM3210C_EVAL_BSP_VERSION_SUB2 (0x02) /*!< [15:8]  sub2 version */
#define __STM3210C_EVAL_BSP_VERSION_RC (0x00)   /*!< [7:0]  release candidate */
#define __STM3210C_EVAL_BSP_VERSION ((__STM3210C_EVAL_BSP_VERSION_MAIN << 24) | (__STM3210C_EVAL_BSP_VERSION_SUB1 << 16) | (__STM3210C_EVAL_BSP_VERSION_SUB2 << 8) | (__STM3210C_EVAL_BSP_VERSION_RC))

/* Note: LCD /CS is CE4 - Bank 4 of NOR/SRAM Bank 1~4 */
#define TFT_LCD_BASE ((uint32_t)(0x60000000 | 0x0C000000))
#define TFT_LCD ((TFT_LCD_TypeDef *)TFT_LCD_BASE)

/**
  * @}
  */

/** @defgroup STM3210C_EVAL_Private_Variables Private Variables
  * @{
  */
/**
 * @brief LED variables
 */

#if defined(BSP_V500_TEST)
GPIO_TypeDef *LED_PORT[LEDn] = {LED1_GPIO_PORT,
                                LED2_GPIO_PORT,
                                LED3_GPIO_PORT};

const uint16_t LED_PIN[LEDn] = {LED1_PIN,
                                LED2_PIN,
                                LED3_PIN};
#else
GPIO_TypeDef *LED_PORT[LEDn] = {LED1_GPIO_PORT,
                                LED2_GPIO_PORT};

const uint16_t LED_PIN[LEDn] = {LED1_PIN,
                                LED2_PIN};
#endif

/**
 * @brief BUTTON variables
 */
GPIO_TypeDef *INPUT_PORT[INPUTn] = {FACTORY_INPUT_GPIO_PORT};

const uint16_t INPUT_PIN[INPUTn] = {FACTORY_INPUT_PIN};

const uint16_t INPUT_IRQn[INPUTn] = {FACTORY_INPUT_EXTI_IRQn};

#if 0
/**
 * @brief BUTTON variables
 */
GPIO_TypeDef *BUTTON_PORT[BUTTONn] = {WAKEUP_BUTTON_GPIO_PORT,
                                      TAMPER_BUTTON_GPIO_PORT,
                                      KEY_BUTTON_GPIO_PORT};

const uint16_t BUTTON_PIN[BUTTONn] = {WAKEUP_BUTTON_PIN,
                                      TAMPER_BUTTON_PIN,
                                      KEY_BUTTON_PIN};

const uint16_t BUTTON_IRQn[BUTTONn] = {WAKEUP_BUTTON_EXTI_IRQn,
                                       TAMPER_BUTTON_EXTI_IRQn,
                                       KEY_BUTTON_EXTI_IRQn};
#endif

#define WDI_HOLD_PIN                    GPIO_PIN_0
#define WDI_HOLD_GPIO_PORT		          GPIOC
#define WDI_HOLD_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define WDI_HOLD_GPIO_CLK_DISBLE()      __HAL_RCC_GPIOC_CLK_DISABLE()

#define PS_HOLD_PIN                    	GPIO_PIN_1
#define PS_HOLD_GPIO_PORT              	GPIOC
#define PS_HOLD_GPIO_CLK_ENABLE()    		__HAL_RCC_GPIOC_CLK_ENABLE()
#define PS_HOLD_GPIO_CLK_DISABLE()    	__HAL_RCC_GPIOC_CLK_DISABLE()

#define WDE_PIN                    			GPIO_PIN_13
#define WDE_GPIO_PORT              			GPIOB
#define WDE_GPIO_CLK_ENABLE()    			  __HAL_RCC_GPIOB_CLK_ENABLE()
#define WDE_GPIO_CLK_DISABLE()    		  __HAL_RCC_GPIOB_CLK_DISABLE()

#define GPS_POWER_EN_PIN             		GPIO_PIN_1
#define GPS_POWER_EN_GPIO_PORT       		GPIOB
#define GPS_POWER_EN_CLK_ENABLE()  		  __HAL_RCC_GPIOB_CLK_ENABLE()
#define GPS_POWER_EN_CLK_DISABLE()  	  __HAL_RCC_GPIOB_CLK_DISABLE()

#define LORA_POWER_EN_PIN             	GPIO_PIN_3
#define LORA_POWER_EN_GPIO_PORT       	GPIOC
#define LORA_POWER_EN_CLK_ENABLE()  		__HAL_RCC_GPIOB_CLK_ENABLE()
#define LORA_POWER_EN_CLK_DISABLE()  		__HAL_RCC_GPIOB_CLK_DISABLE()

#define LORA_RESET_EN_PIN             	GPIO_PIN_2
#define LORA_RESET_EN_GPIO_PORT       	GPIOC
#define LORA_RESET_EN_CLK_ENABLE()  		__HAL_RCC_GPIOB_CLK_ENABLE()
#define LORA_RESET_EN_CLK_DISABLE()  		__HAL_RCC_GPIOB_CLK_DISABLE()

#if defined(BSP_V500_TEST)
GPIO_TypeDef *OUTPUT_PORT[OUTGPIOn] = {WDI_HOLD_GPIO_PORT,
                                      PS_HOLD_GPIO_PORT,
                                      WDE_GPIO_PORT,
                                      GPS_POWER_EN_GPIO_PORT,
                                      LORA_POWER_EN_GPIO_PORT,
                                      LORA_RESET_EN_GPIO_PORT};

const uint16_t OUTPUT_PIN[OUTGPIOn] = {WDI_HOLD_PIN,
                                        PS_HOLD_PIN,
                                        WDE_PIN,
                                        GPS_POWER_EN_PIN,
                                        LORA_POWER_EN_PIN,
                                        LORA_RESET_EN_PIN};

#else
GPIO_TypeDef *OUTPUT_PORT[OUTGPIOn] = {WKUP_GPIO_PORT,
                                      NRST_GPIO_PORT,
                                      PRST_GPIO_PORT};

const uint16_t OUTPUT_PIN[OUTGPIOn] = {WKUP_PIN,
                                      NRST_PIN,
                                      PRST_PIN};

#endif
/**
 * @brief COM variables
 */
USART_TypeDef *COM_USART[COMn] = {EVAL_COM1};

GPIO_TypeDef *COM_TX_PORT[COMn] = {EVAL_COM1_TX_GPIO_PORT};

GPIO_TypeDef *COM_RX_PORT[COMn] = {EVAL_COM1_RX_GPIO_PORT};

const uint16_t COM_TX_PIN[COMn] = {EVAL_COM1_TX_PIN};

const uint16_t COM_RX_PIN[COMn] = {EVAL_COM1_RX_PIN};

/**
 * @brief BUS variables
 */
#ifdef HAL_SPI_MODULE_ENABLED
uint32_t SpixTimeout = EVAL_SPIx_TIMEOUT_MAX; /*<! Value of Timeout when SPI communication fails */
static SPI_HandleTypeDef heval_Spi;
#endif /* HAL_SPI_MODULE_ENABLED */

#ifdef HAL_I2C_MODULE_ENABLED
uint32_t I2cxTimeout = EVAL_I2Cx_TIMEOUT_MAX; /*<! Value of Timeout when I2C communication fails */
I2C_HandleTypeDef heval_I2c;
#endif /* HAL_I2C_MODULE_ENABLED */

/**
  * @}
  */

/* I2Cx bus function */
#ifdef HAL_I2C_MODULE_ENABLED
/* Link function for I2C EEPROM peripheral */
static void I2Cx_Init(void);
static void I2Cx_ITConfig(void);
static HAL_StatusTypeDef I2Cx_ReadMultiple(uint8_t Addr, uint16_t Reg, uint16_t MemAddress, uint8_t *Buffer, uint16_t Length);
static HAL_StatusTypeDef I2Cx_ReadBuffer(uint16_t Addr, uint8_t Reg, uint16_t RegSize, uint8_t *pBuffer, uint16_t Length);
static void I2Cx_WriteData(uint16_t Addr, uint8_t Reg, uint8_t Value);
static HAL_StatusTypeDef I2Cx_WriteBuffer(uint16_t Addr, uint8_t Reg, uint16_t RegSize, uint8_t *pBuffer, uint16_t Length);
static uint8_t I2Cx_ReadData(uint16_t Addr, uint8_t Reg);
static HAL_StatusTypeDef I2Cx_IsDeviceReady(uint16_t DevAddress, uint32_t Trials);
static void I2Cx_Error(uint8_t Addr);
static void I2Cx_MspInit(I2C_HandleTypeDef *hi2c);

/* Link function for IO Expander over I2C */
void IOE_Init(void);
void IOE_ITConfig(void);
void IOE_Delay(uint32_t Delay);
void IOE_Write(uint8_t Addr, uint8_t Reg, uint8_t Value);
uint8_t IOE_Read(uint8_t Addr, uint8_t Reg);
uint16_t IOE_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length);

/* Link function for EEPROM peripheral over I2C */
void EEPROM_I2C_IO_Init(void);
HAL_StatusTypeDef EEPROM_I2C_IO_WriteData(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pBuffer, uint32_t BufferSize);
HAL_StatusTypeDef EEPROM_I2C_IO_ReadData(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pBuffer, uint32_t BufferSize);
HAL_StatusTypeDef EEPROM_I2C_IO_IsDeviceReady(uint16_t DevAddress, uint32_t Trials);

/* Link functions for Temperature Sensor peripheral */
void TSENSOR_IO_Init(void);
void TSENSOR_IO_Write(uint16_t DevAddress, uint8_t *pBuffer, uint8_t WriteAddr, uint16_t Length);
void TSENSOR_IO_Read(uint16_t DevAddress, uint8_t *pBuffer, uint8_t ReadAddr, uint16_t Length);
uint16_t TSENSOR_IO_IsDeviceReady(uint16_t DevAddress, uint32_t Trials);

/* Link function for Audio peripheral */
void AUDIO_IO_Init(void);
void AUDIO_IO_DeInit(void);
void AUDIO_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value);
uint8_t AUDIO_IO_Read(uint8_t Addr, uint8_t Reg);

/* Link function for Accelero peripheral */
void ACCELERO_IO_Init(void);
void ACCELERO_IO_ITConfig(void);
void ACCELERO_IO_Write(uint8_t *pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite);
void ACCELERO_IO_Read(uint8_t *pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead);

#endif /* HAL_I2C_MODULE_ENABLED */

/* SPIx bus function */
#ifdef HAL_SPI_MODULE_ENABLED
static void SPIx_Init(void);
static void SPIx_Write(uint8_t Value);
static uint32_t SPIx_Read(void);
static void SPIx_Error(void);
static void SPIx_MspInit(SPI_HandleTypeDef *hspi);

/* Link function for LCD peripheral over SPI */
void LCD_IO_Init(void);
void LCD_IO_WriteMultipleData(uint8_t *pData, uint32_t Size);
void LCD_IO_WriteReg(uint8_t Reg);
uint16_t LCD_IO_ReadData(uint16_t RegValue);
void LCD_Delay(uint32_t delay);

/* Link functions for SD Card peripheral over SPI */
void SD_IO_Init(void);
HAL_StatusTypeDef SD_IO_WriteCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc, uint8_t Response);
HAL_StatusTypeDef SD_IO_WaitResponse(uint8_t Response);
void SD_IO_WriteDummy(void);
void SD_IO_WriteByte(uint8_t Data);
uint8_t SD_IO_ReadByte(void);

#endif /* HAL_SPI_MODULE_ENABLED */

/** @defgroup STM3210C_EVAL_Exported_Functions Exported Functions
  * @{
  */

char BSP_GetBootloaderVersion(void)
{
    return __BOOTLOADER_VERSION;
}

char BSP_GetAppVersion(void)
{
    return __APPLICATION_VERSION;
}


/**
  * @brief  This method returns the STM3210C EVAL BSP Driver revision
  * @retval version : 0xXYZR (8bits for each decimal, R for RC)
  */
uint32_t BSP_GetVersion(void)
{
    return __STM3210C_EVAL_BSP_VERSION;
}

/**
  * @brief  Configures LED GPIO.
  * @param  Led: Specifies the Led to be configured. 
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4
  * @retval None
  */
void BSP_LED_Init(Led_TypeDef Led)
{
    GPIO_InitTypeDef gpioinitstruct = {0};

    /* Enable the GPIO_LED clock */
    LEDx_GPIO_CLK_ENABLE(Led);

    /* Configure the GPIO_LED pin */
    gpioinitstruct.Pin = LED_PIN[Led];
    gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
    gpioinitstruct.Pull = GPIO_NOPULL;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(LED_PORT[Led], &gpioinitstruct);

    HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);
}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on. 
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4
  * @retval None
  */
void BSP_LED_On(Led_TypeDef Led)
{
    HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_SET);
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off. 
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4
  * @retval None
  */
void BSP_LED_Off(Led_TypeDef Led)
{
    HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled. 
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4
  * @retval None
  */
#define SIGNAUTURE_EMERGENCE    0x69696969

void BSP_LED_Toggle(Led_TypeDef Led)
{
    HAL_GPIO_TogglePin(LED_PORT[Led], LED_PIN[Led]);
}

bool BSP_IsDownloadModeActivated(void)
{
    uint32_t Address = SP_FLASH_START_ADDRESS;

    if((*(__IO uint32_t *)Address) == SIGNAUTURE_EMERGENCE)
    {
      return true;
    }
    
    return false;
}

void BSP_Download_Reset(void)
{
    __IO uint32_t FLASHStatus = FLASHIF_OK;
    uint32_t Address = SP_FLASH_START_ADDRESS;
    uint32_t signature = SIGNAUTURE_EMERGENCE;
    int retry = 3;
    
    /* Unlock the Program memory */
    HAL_FLASH_Unlock();

    /* Clear all FLASH flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

    /* Unlock the Program memory */
    HAL_FLASH_Lock();
    
    FLASHStatus = FLASH_If_GPS_Erase_Page(Address);
    if (FLASHStatus != FLASHIF_OK)
    {
        printf("  the erase process is failed...\r\n");
    }
    else
    {
      while(retry--) {
        FLASHStatus = FLASH_If_Write(Address, &signature, 1);
        if (FLASHStatus == FLASHIF_OK)
        {
            HAL_NVIC_SystemReset();
        }
        else
        {
            /* Unlock the Program memory */
            HAL_FLASH_Unlock();
            
            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
            
            /* Unlock the Program memory */
            HAL_FLASH_Lock();
          
        }
      }   
        
    }
}

void BSP_Booting_Reset(void)
{
    __IO uint32_t FLASHStatus = FLASHIF_OK;
    uint32_t Address = SP_FLASH_START_ADDRESS;
    int signature = 0;
    int retry = 3;
    
    /* Unlock the Program memory */
    HAL_FLASH_Unlock();

    /* Clear all FLASH flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

    /* Unlock the Program memory */
    HAL_FLASH_Lock();
    
    FLASHStatus = FLASH_If_GPS_Erase_Page(Address);
    if (FLASHStatus != FLASHIF_OK)
    {
        printf("  the erase process is failed...\r\n");
    }
    else
    {
      while(retry--) {
        FLASHStatus = FLASH_If_Write(Address, &signature, 1);
        if (FLASHStatus == FLASHIF_OK)
        {
            HAL_NVIC_SystemReset();
        }
        else
        {
            /* Unlock the Program memory */
            HAL_FLASH_Unlock();
            
            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
            
            /* Unlock the Program memory */
            HAL_FLASH_Lock();
          
        }
      }   
        
    }
}

void BSP_HW_Reset(void)
{
    BSP_OUTGPIO_High(OUTPUT_PRST);
    BSP_Lora_HW_Reset();
    HAL_NVIC_SystemReset();  
}

void BSP_Delay_HW_Reset(void)
{
    HAL_Delay(5000);
    BSP_HW_Reset();
}

void BSP_Lora_HW_Reset(void)
{
    BSP_OUTGPIO_Low(OUTPUT_NRST);
    HAL_Delay(10);
    BSP_OUTGPIO_High(OUTPUT_NRST);
}

void BSP_Lora_Wakeup(void)
{
    BSP_OUTGPIO_High(OUTPUT_WKUP);
    HAL_Delay(2);
    BSP_OUTGPIO_Low(OUTPUT_WKUP);
}



/**
  * @brief  Configures Outpu GPIO.
  * @param  Out: Specifies the output to be configured. 
  *   This parameter can be one of following parameters:
  * @retval None
  */
void BSP_OUTGPIO_init(Output_TypeDef Outpin, GPIO_PinState initialstatus)
{
    GPIO_InitTypeDef gpioinitstruct = {0};

    /* Enable the corresponding Push Button clock */
    OUTPUTx_GPIO_CLK_ENABLE(Outpin);

    /* Configure Push Button pin as input */
    gpioinitstruct.Pin = OUTPUT_PIN[Outpin];
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;
    gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(OUTPUT_PORT[Outpin], &gpioinitstruct);

    HAL_GPIO_WritePin(OUTPUT_PORT[Outpin], OUTPUT_PIN[Outpin], initialstatus);
}

void BSP_OUTGPIO_Low(Output_TypeDef Outpin)
{
    HAL_GPIO_WritePin(OUTPUT_PORT[Outpin], OUTPUT_PIN[Outpin], GPIO_PIN_RESET);
}

void BSP_OUTGPIO_High(Output_TypeDef Outpin)
{
    HAL_GPIO_WritePin(OUTPUT_PORT[Outpin], OUTPUT_PIN[Outpin], GPIO_PIN_SET);
}


    /**
  * @brief  Configures push input GPIO and EXTI Line.
  * @param  input: input to be configured.
  *   This parameter can be one of the following values: 
  *     @arg INPUT_FACTORY: factory input 
  * @param  Input_Mode: Input mode requested.
  *   This parameter can be one of the following values:   
  *     @arg INTPUT_MODE_GPIO: Input will be used as simple IO 
  *     @arg INTPUT_MODE_EXTI: Input will be connected to EXTI line
  *                            with interrupt generation capability
  * @retval None
  */
void BSP_Input_Init(Input_TypeDef InputPin, InputMode_TypeDef Input_Mode)
{
    GPIO_InitTypeDef gpioinitstruct = {0};

    /* Enable the corresponding input Pin clock */
    INPUTx_GPIO_CLK_ENABLE(InputPin);

    /* Configure Push input pin as input */
    gpioinitstruct.Pin = INPUT_PIN[InputPin];
    gpioinitstruct.Pull = GPIO_NOPULL;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;

    if (Input_Mode == INPUT_MODE_GPIO)
    {
        /* Configure input pin as input */
        gpioinitstruct.Mode = GPIO_MODE_INPUT;
        HAL_GPIO_Init(INPUT_PORT[InputPin], &gpioinitstruct);
    }
    else if (Input_Mode == INPUT_MODE_EXTI)
    {
        if (InputPin != INPUT_FACTORY)
        {
            /* Configure Joystick input pin as input with External interrupt, falling edge */
            gpioinitstruct.Mode = GPIO_MODE_IT_FALLING;
        }
        else
        {
            /* Configure Key Push input pin as input with External interrupt, rising edge */
            gpioinitstruct.Mode = GPIO_MODE_IT_RISING;
        }
        HAL_GPIO_Init(INPUT_PORT[InputPin], &gpioinitstruct);

        /* Enable and set input EXTI Interrupt to the lowest priority */
        HAL_NVIC_SetPriority((IRQn_Type)(INPUT_IRQn[InputPin]), 0x0F, 0);
        HAL_NVIC_EnableIRQ((IRQn_Type)(INPUT_IRQn[InputPin]));
    }
}

/**
  * @brief  Returns the selected input state.
  * @param  input: input to be checked.
  *   This parameter can be one of the following values:
  *     @arg BUTTON_TAMPER: Key/Tamper Push Button 
  * @retval input state
  */
uint32_t BSP_Input_GetState(Input_TypeDef Input)
{
    return HAL_GPIO_ReadPin(INPUT_PORT[Input], INPUT_PIN[Input]);
}

#if 0
    /**
  * @brief  Configures push button GPIO and EXTI Line.
  * @param  Button: Button to be configured.
  *   This parameter can be one of the following values: 
  *     @arg BUTTON_WAKEUP: Wakeup Push Button 
  *     @arg BUTTON_TAMPER: Tamper Push Button 
  *     @arg BUTTON_KEY: Key Push Button 
  * @param  Button_Mode: Button mode requested.
  *   This parameter can be one of the following values:   
  *     @arg BUTTON_MODE_GPIO: Button will be used as simple IO 
  *     @arg BUTTON_MODE_EXTI: Button will be connected to EXTI line
  *                            with interrupt generation capability
  * @retval None
  */
void BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode)
{
    GPIO_InitTypeDef gpioinitstruct = {0};

    /* Enable the corresponding Push Button clock */
    BUTTONx_GPIO_CLK_ENABLE(Button);

    /* Configure Push Button pin as input */
    gpioinitstruct.Pin = BUTTON_PIN[Button];
    gpioinitstruct.Pull = GPIO_NOPULL;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;

    if (Button_Mode == BUTTON_MODE_GPIO)
    {
        /* Configure Button pin as input */
        gpioinitstruct.Mode = GPIO_MODE_INPUT;
        HAL_GPIO_Init(BUTTON_PORT[Button], &gpioinitstruct);
    }
    else if (Button_Mode == BUTTON_MODE_EXTI)
    {
        if (Button != BUTTON_WAKEUP)
        {
            /* Configure Joystick Button pin as input with External interrupt, falling edge */
            gpioinitstruct.Mode = GPIO_MODE_IT_FALLING;
        }
        else
        {
            /* Configure Key Push Button pin as input with External interrupt, rising edge */
            gpioinitstruct.Mode = GPIO_MODE_IT_RISING;
        }
        HAL_GPIO_Init(BUTTON_PORT[Button], &gpioinitstruct);

        /* Enable and set Button EXTI Interrupt to the lowest priority */
        HAL_NVIC_SetPriority((IRQn_Type)(BUTTON_IRQn[Button]), 0x0F, 0);
        HAL_NVIC_EnableIRQ((IRQn_Type)(BUTTON_IRQn[Button]));
    }
}

/**
  * @brief  Returns the selected button state.
  * @param  Button: Button to be checked.
  *   This parameter can be one of the following values:
  *     @arg BUTTON_TAMPER: Key/Tamper Push Button 
  * @retval Button state
  */
uint32_t BSP_PB_GetState(Button_TypeDef Button)
{
    return HAL_GPIO_ReadPin(BUTTON_PORT[Button], BUTTON_PIN[Button]);
}
#endif

#ifdef HAL_I2C_MODULE_ENABLED
#if 0
/**
  * @brief  Configures joystick GPIO and EXTI modes.
  * @param  Joy_Mode: Button mode.
  *          This parameter can be one of the following values:
  *            @arg  JOY_MODE_GPIO: Joystick pins will be used as simple IOs
  *            @arg  JOY_MODE_EXTI: Joystick pins will be connected to EXTI line 
  *                                 with interrupt generation capability  
  * @retval IO_OK: if all initializations are OK. Other value if error.
  */
uint8_t BSP_JOY_Init(JOYMode_TypeDef Joy_Mode)
{
    uint8_t ret = 0;

    /* Initialize the IO functionalities */
    ret = BSP_IO_Init();

    /* Configure joystick pins in IT mode */
    if ((ret == IO_OK) && (Joy_Mode == JOY_MODE_EXTI))
    {
        /* Configure joystick pins in IT mode */
        BSP_IO_ConfigPin(JOY_ALL_PINS, IO_MODE_IT_FALLING_EDGE);
    }

    return ret;
}

/**
  * @brief  Returns the current joystick status.
  * @retval Code of the joystick key pressed
  *          This code can be one of the following values:
  *            @arg  JOY_NONE
  *            @arg  JOY_SEL
  *            @arg  JOY_DOWN
  *            @arg  JOY_LEFT
  *            @arg  JOY_RIGHT
  *            @arg  JOY_UP
  */
JOYState_TypeDef BSP_JOY_GetState(void)
{
    uint32_t tmp = 0;

    /* Read the status joystick pins */
    tmp = BSP_IO_ReadPin(JOY_ALL_PINS);

    /* Check the pressed keys */
    if ((tmp & JOY_NONE_PIN) == JOY_NONE)
    {
        return (JOYState_TypeDef)JOY_NONE;
    }
    else if (!(tmp & JOY_SEL_PIN))
    {
        return (JOYState_TypeDef)JOY_SEL;
    }
    else if (!(tmp & JOY_DOWN_PIN))
    {
        return (JOYState_TypeDef)JOY_DOWN;
    }
    else if (!(tmp & JOY_LEFT_PIN))
    {
        return (JOYState_TypeDef)JOY_LEFT;
    }
    else if (!(tmp & JOY_RIGHT_PIN))
    {
        return (JOYState_TypeDef)JOY_RIGHT;
    }
    else if (!(tmp & JOY_UP_PIN))
    {
        return (JOYState_TypeDef)JOY_UP;
    }
    else
    {
        return (JOYState_TypeDef)JOY_NONE;
    }
}
#endif

#endif /*HAL_I2C_MODULE_ENABLED*/

#ifdef HAL_UART_MODULE_ENABLED
/**
  * @brief  Configures COM port.
  * @param  COM: Specifies the COM port to be configured.
  *   This parameter can be one of following parameters:    
  *     @arg COM1
  * @param  huart: pointer to a UART_HandleTypeDef structure that
  *   contains the configuration information for the specified UART peripheral.
  * @retval None
  */
void BSP_COM_Init(COM_TypeDef COM, UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpioinitstruct = {0};

    /* Enable GPIO clock */
    COMx_TX_GPIO_CLK_ENABLE(COM);
    COMx_RX_GPIO_CLK_ENABLE(COM);

    /* Enable USART clock */
    COMx_CLK_ENABLE(COM);

    /* Remap AFIO if needed */
    AFIOCOMx_CLK_ENABLE(COM);
    AFIOCOMx_REMAP(COM);

    /* Configure USART Tx as alternate function push-pull */
    gpioinitstruct.Pin = COM_TX_PIN[COM];
    gpioinitstruct.Mode = GPIO_MODE_AF_PP;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;
    gpioinitstruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(COM_TX_PORT[COM], &gpioinitstruct);

    /* Configure USART Rx as alternate function push-pull */
    gpioinitstruct.Mode = GPIO_MODE_INPUT;
    gpioinitstruct.Pin = COM_RX_PIN[COM];
    HAL_GPIO_Init(COM_RX_PORT[COM], &gpioinitstruct);

    /* USART configuration */
    huart->Instance = COM_USART[COM];
    HAL_UART_Init(huart);
}
#endif /* HAL_UART_MODULE_ENABLED */

/**
  * @}
  */

/** @defgroup STM3210C_EVAL_BusOperations_Functions Bus Operations Functions
  * @{
  */

/*******************************************************************************
                            BUS OPERATIONS
*******************************************************************************/

#ifdef HAL_I2C_MODULE_ENABLED
/******************************* I2C Routines**********************************/

/**
  * @brief Eval I2Cx MSP Initialization
  * @param hi2c: I2C handle
  * @retval None
  */
static void I2Cx_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef gpioinitstruct = {0};

    if (hi2c->Instance == EVAL_I2Cx)
    {
        /*## Configure the GPIOs ################################################*/

        /* Enable GPIO clock */
        EVAL_I2Cx_SDA_GPIO_CLK_ENABLE();
        EVAL_I2Cx_SCL_GPIO_CLK_ENABLE();

        /* Configure I2C Tx as alternate function  */
        gpioinitstruct.Pin = EVAL_I2Cx_SCL_PIN;
        gpioinitstruct.Mode = GPIO_MODE_AF_OD;
        gpioinitstruct.Pull = GPIO_NOPULL;
        gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(EVAL_I2Cx_SCL_GPIO_PORT, &gpioinitstruct);

        /* Configure I2C Rx as alternate function  */
        gpioinitstruct.Pin = EVAL_I2Cx_SDA_PIN;
        HAL_GPIO_Init(EVAL_I2Cx_SDA_GPIO_PORT, &gpioinitstruct);

        /*## Configure the Eval I2Cx peripheral #######################################*/
        /* Enable Eval_I2Cx clock */
        EVAL_I2Cx_CLK_ENABLE();

        /* Add delay related to RCC workaround */
        while (READ_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C1EN) != RCC_APB1ENR_I2C1EN)
        {
        };

        /* Force the I2C Periheral Clock Reset */
        EVAL_I2Cx_FORCE_RESET();

        /* Release the I2C Periheral Clock Reset */
        EVAL_I2Cx_RELEASE_RESET();

        /* Enable and set Eval I2Cx Interrupt to the highest priority */
        HAL_NVIC_SetPriority(EVAL_I2Cx_EV_IRQn, 0xE, 0);
        HAL_NVIC_EnableIRQ(EVAL_I2Cx_EV_IRQn);

        /* Enable and set Eval I2Cx Interrupt to the highest priority */
        HAL_NVIC_SetPriority(EVAL_I2Cx_ER_IRQn, 0xE, 0);
        HAL_NVIC_EnableIRQ(EVAL_I2Cx_ER_IRQn);
    }
}

/**
  * @brief Eval I2Cx Bus initialization
  * @retval None
  */
static void I2Cx_Init(void)
{
    if (HAL_I2C_GetState(&heval_I2c) == HAL_I2C_STATE_RESET)
    {
        heval_I2c.Instance = EVAL_I2Cx;
        heval_I2c.Init.ClockSpeed = BSP_I2C_SPEED;
        heval_I2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
        heval_I2c.Init.OwnAddress1 = 0;
        heval_I2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
        heval_I2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
        heval_I2c.Init.OwnAddress2 = 0;
        heval_I2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
        heval_I2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

        /* Init the I2C */
        I2Cx_MspInit(&heval_I2c);
        HAL_I2C_Init(&heval_I2c);
    }
}

/**
  * @brief  Configures I2C Interrupt.
  * @retval None
  */
static void I2Cx_ITConfig(void)
{
    static uint8_t I2C_IT_Enabled = 0;
    GPIO_InitTypeDef gpioinitstruct = {0};

    if (I2C_IT_Enabled == 0)
    {
        I2C_IT_Enabled = 1;

        /* Enable the GPIO EXTI clock */
        IOE_IT_GPIO_CLK_ENABLE();

        gpioinitstruct.Pin = IOE_IT_PIN;
        gpioinitstruct.Pull = GPIO_NOPULL;
        gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;
        gpioinitstruct.Mode = GPIO_MODE_IT_FALLING;
        HAL_GPIO_Init(IOE_IT_GPIO_PORT, &gpioinitstruct);

        /* Set priority and Enable GPIO EXTI Interrupt */
        HAL_NVIC_SetPriority((IRQn_Type)(IOE_IT_EXTI_IRQn), 0xE, 0);
        HAL_NVIC_EnableIRQ((IRQn_Type)(IOE_IT_EXTI_IRQn));
    }
}

/**
  * @brief  Reads multiple data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @param  MemAddress: Internal memory address
  * @param  Buffer: Pointer to data buffer
  * @param  Length: Length of the data
  * @retval Number of read data
  */
static HAL_StatusTypeDef I2Cx_ReadMultiple(uint8_t Addr, uint16_t Reg, uint16_t MemAddress, uint8_t *Buffer, uint16_t Length)
{
    HAL_StatusTypeDef status = HAL_OK;

    status = HAL_I2C_Mem_Read(&heval_I2c, Addr, (uint16_t)Reg, MemAddress, Buffer, Length, I2cxTimeout);

    /* Check the communication status */
    if (status != HAL_OK)
    {
        /* I2C error occured */
        I2Cx_Error(Addr);
    }
    return status;
}

/**
  * @brief  Write a value in a register of the device through BUS.
  * @param  Addr: Device address on BUS Bus.  
  * @param  Reg: The target register address to write
  * @param  Value: The target register value to be written 
  * @retval  None
  */
static void I2Cx_WriteData(uint16_t Addr, uint8_t Reg, uint8_t Value)
{
    HAL_StatusTypeDef status = HAL_OK;

    status = HAL_I2C_Mem_Write(&heval_I2c, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, I2cxTimeout);

    /* Check the communication status */
    if (status != HAL_OK)
    {
        /* Execute user timeout callback */
        I2Cx_Error(Addr);
    }
}

/**
  * @brief  Write a value in a register of the device through BUS.
  * @param  Addr: Device address on BUS Bus.  
  * @param  Reg: The target register address to write
  * @param  RegSize: The target register size (can be 8BIT or 16BIT)
  * @param  pBuffer: The target register value to be written 
  * @param  Length: buffer size to be written
  * @retval None
  */
static HAL_StatusTypeDef I2Cx_WriteBuffer(uint16_t Addr, uint8_t Reg, uint16_t RegSize, uint8_t *pBuffer, uint16_t Length)
{
    HAL_StatusTypeDef status = HAL_OK;

    status = HAL_I2C_Mem_Write(&heval_I2c, Addr, (uint16_t)Reg, RegSize, pBuffer, Length, I2cxTimeout);

    /* Check the communication status */
    if (status != HAL_OK)
    {
        /* Re-Initiaize the BUS */
        I2Cx_Error(Addr);
    }
    return status;
}

/**
  * @brief  Read a value in a register of the device through BUS.
  * @param  Addr: Device address on BUS Bus.  
  * @param  Reg: The target register address to write
  * @retval Data read at register @
  */
static uint8_t I2Cx_ReadData(uint16_t Addr, uint8_t Reg)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t value = 0;

    status = HAL_I2C_Mem_Read(&heval_I2c, Addr, Reg, I2C_MEMADD_SIZE_8BIT, &value, 1, I2cxTimeout);

    /* Check the communication status */
    if (status != HAL_OK)
    {
        /* Execute user timeout callback */
        I2Cx_Error(Addr);
    }
    return value;
}

/**
  * @brief  Reads multiple data on the BUS.
  * @param  Addr: I2C Address
  * @param  Reg: Reg Address 
  * @param  RegSize : The target register size (can be 8BIT or 16BIT)
  * @param  pBuffer: pointer to read data buffer
  * @param  Length: length of the data
  * @retval 0 if no problems to read multiple data
  */
static HAL_StatusTypeDef I2Cx_ReadBuffer(uint16_t Addr, uint8_t Reg, uint16_t RegSize, uint8_t *pBuffer, uint16_t Length)
{
    HAL_StatusTypeDef status = HAL_OK;

    status = HAL_I2C_Mem_Read(&heval_I2c, Addr, (uint16_t)Reg, RegSize, pBuffer, Length, I2cxTimeout);

    /* Check the communication status */
    if (status != HAL_OK)
    {
        /* Re-Initiaize the BUS */
        I2Cx_Error(Addr);
    }
    return status;
}

/**
* @brief  Checks if target device is ready for communication. 
* @note   This function is used with Memory devices
* @param  DevAddress: Target device address
* @param  Trials: Number of trials
* @retval HAL status
*/
static HAL_StatusTypeDef I2Cx_IsDeviceReady(uint16_t DevAddress, uint32_t Trials)
{
    return (HAL_I2C_IsDeviceReady(&heval_I2c, DevAddress, Trials, I2cxTimeout));
}

/**
  * @brief  Manages error callback by re-initializing I2C.
  * @param  Addr: I2C Address
  * @retval None
  */
static void I2Cx_Error(uint8_t Addr)
{
    /* De-initialize the IOE comunication BUS */
    HAL_I2C_DeInit(&heval_I2c);

    /* Re-Initiaize the IOE comunication BUS */
    I2Cx_Init();
}

#endif /* HAL_I2C_MODULE_ENABLED */

/******************************* SPI Routines**********************************/
#ifdef HAL_SPI_MODULE_ENABLED
/**
  * @brief  Initializes SPI MSP.
  * @retval None
  */
static void SPIx_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef gpioinitstruct = {0};

    /*** Configure the GPIOs ***/
    /* Enable GPIO clock */
    EVAL_SPIx_SCK_GPIO_CLK_ENABLE();
    EVAL_SPIx_MISO_MOSI_GPIO_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_SPI3_ENABLE();

    /* configure SPI SCK */
    gpioinitstruct.Pin = EVAL_SPIx_SCK_PIN;
    gpioinitstruct.Mode = GPIO_MODE_AF_PP;
    gpioinitstruct.Pull = GPIO_NOPULL;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(EVAL_SPIx_SCK_GPIO_PORT, &gpioinitstruct);

    /* configure SPI MISO and MOSI */
    gpioinitstruct.Pin = (EVAL_SPIx_MISO_PIN | EVAL_SPIx_MOSI_PIN);
    gpioinitstruct.Mode = GPIO_MODE_AF_PP;
    gpioinitstruct.Pull = GPIO_NOPULL;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(EVAL_SPIx_MISO_MOSI_GPIO_PORT, &gpioinitstruct);

    /*** Configure the SPI peripheral ***/
    /* Enable SPI clock */
    EVAL_SPIx_CLK_ENABLE();
}

/**
  * @brief  Initializes SPI HAL.
  * @retval None
  */
static void SPIx_Init(void)
{
    /* DeInitializes the SPI peripheral */
    heval_Spi.Instance = EVAL_SPIx;
    HAL_SPI_DeInit(&heval_Spi);

    /* SPI Config */
    /* SPI baudrate is set to 9 MHz (PCLK2/SPI_BaudRatePrescaler = 72/8 = 9 MHz) */
    heval_Spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    heval_Spi.Init.Direction = SPI_DIRECTION_2LINES;
    heval_Spi.Init.CLKPhase = SPI_PHASE_2EDGE;
    heval_Spi.Init.CLKPolarity = SPI_POLARITY_HIGH;
    heval_Spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    heval_Spi.Init.CRCPolynomial = 7;
    heval_Spi.Init.DataSize = SPI_DATASIZE_8BIT;
    heval_Spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    heval_Spi.Init.NSS = SPI_NSS_SOFT;
    heval_Spi.Init.TIMode = SPI_TIMODE_DISABLE;
    heval_Spi.Init.Mode = SPI_MODE_MASTER;

    SPIx_MspInit(&heval_Spi);
    if (HAL_SPI_Init(&heval_Spi) != HAL_OK)
    {
        /* Should not occur */
        while (1)
        {
        };
    }
}

/**
  * @brief SPI Read 4 bytes from device
  * @retval Read data
*/
static uint32_t SPIx_Read(void)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t readvalue = 0;
    uint32_t writevalue = 0xFFFFFFFF;

    status = HAL_SPI_TransmitReceive(&heval_Spi, (uint8_t *)&writevalue, (uint8_t *)&readvalue, 1, SpixTimeout);

    /* Check the communication status */
    if (status != HAL_OK)
    {
        /* Execute user timeout callback */
        SPIx_Error();
    }

    return readvalue;
}

/**
  * @brief SPI Write a byte to device
  * @param Value: value to be written
  * @retval None
  */
static void SPIx_Write(uint8_t Value)
{
    HAL_StatusTypeDef status = HAL_OK;

    status = HAL_SPI_Transmit(&heval_Spi, (uint8_t *)&Value, 1, SpixTimeout);

    /* Check the communication status */
    if (status != HAL_OK)
    {
        /* Execute user timeout callback */
        SPIx_Error();
    }
}

/**
  * @brief SPI error treatment function
  * @retval None
  */
static void SPIx_Error(void)
{
    /* De-initialize the SPI communication BUS */
    HAL_SPI_DeInit(&heval_Spi);

    /* Re- Initiaize the SPI communication BUS */
    SPIx_Init();
}
#endif /* HAL_SPI_MODULE_ENABLED */

/**
  * @}
  */

/** @defgroup STM3210C_EVAL_LinkOperations_Functions Link Operations Functions
  * @{
  */

/*******************************************************************************
                            LINK OPERATIONS
*******************************************************************************/

#ifdef HAL_I2C_MODULE_ENABLED
/***************************** LINK IOE ***************************************/

/**
  * @brief  Initializes IOE low level.
  * @retval None
  */
void IOE_Init(void)
{
    I2Cx_Init();
}

/**
  * @brief  Configures IOE low level Interrupt.
  * @retval None
  */
void IOE_ITConfig(void)
{
    I2Cx_ITConfig();
}

/**
  * @brief  IOE writes single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @param  Value: Data to be written
  * @retval None
  */
void IOE_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
    I2Cx_WriteData(Addr, Reg, Value);
}

/**
  * @brief  IOE reads single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @retval Read data
  */
uint8_t IOE_Read(uint8_t Addr, uint8_t Reg)
{
    return I2Cx_ReadData(Addr, Reg);
}

/**
  * @brief  IOE reads multiple data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @param  Buffer: Pointer to data buffer
  * @param  Length: Length of the data
  * @retval Number of read data
  */
uint16_t IOE_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length)
{
    return I2Cx_ReadMultiple(Addr, Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length);
}

/**
  * @brief  IOE delay. 
  * @param  Delay: Delay in ms
  * @retval None
  */
void IOE_Delay(uint32_t Delay)
{
    HAL_Delay(Delay);
}

#endif /* HAL_I2C_MODULE_ENABLED */

#ifdef HAL_SPI_MODULE_ENABLED
/********************************* LINK LCD ***********************************/

/**
  * @brief  Configures the LCD_SPI interface.
  * @retval None
  */
void LCD_IO_Init(void)
{
    GPIO_InitTypeDef gpioinitstruct;

    /* Configure the LCD Control pins ------------------------------------------*/
    LCD_NCS_GPIO_CLK_ENABLE();

    /* Configure NCS in Output Push-Pull mode */
    gpioinitstruct.Pin = LCD_NCS_PIN;
    gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
    gpioinitstruct.Pull = GPIO_NOPULL;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_NCS_GPIO_PORT, &gpioinitstruct);

    /* Set or Reset the control line */
    LCD_CS_LOW();
    LCD_CS_HIGH();

    SPIx_Init();
}

/**
  * @brief  Write register value.
  * @param  pData Pointer on the register value
  * @param  Size Size of byte to transmit to the register
  * @retval None
  */
void LCD_IO_WriteMultipleData(uint8_t *pData, uint32_t Size)
{
    uint32_t counter = 0;

    /* Reset LCD control line(/CS) and Send data */
    LCD_CS_LOW();

    /* Send Start Byte */
    SPIx_Write(START_BYTE | LCD_WRITE_REG);

    for (counter = Size; counter != 0; counter--)
    {
        while (((heval_Spi.Instance->SR) & SPI_FLAG_TXE) != SPI_FLAG_TXE)
        {
        }
        /* Need to invert bytes for LCD*/
        *((__IO uint8_t *)&heval_Spi.Instance->DR) = *(pData + 1);

        while (((heval_Spi.Instance->SR) & SPI_FLAG_TXE) != SPI_FLAG_TXE)
        {
        }
        *((__IO uint8_t *)&heval_Spi.Instance->DR) = *pData;
        counter--;
        pData += 2;
    }

    /* Wait until the bus is ready before releasing Chip select */
    while (((heval_Spi.Instance->SR) & SPI_FLAG_BSY) != RESET)
    {
    }

    /* Reset LCD control line(/CS) and Send data */
    LCD_CS_HIGH();
}

/**
  * @brief  register address.
  * @param  Reg
  * @retval None
  */
void LCD_IO_WriteReg(uint8_t Reg)
{
    /* Reset LCD control line(/CS) and Send command */
    LCD_CS_LOW();

    /* Send Start Byte */
    SPIx_Write(START_BYTE | SET_INDEX);

    /* Write 16-bit Reg Index (High Byte is 0) */
    SPIx_Write(0x00);
    SPIx_Write(Reg);

    /* Deselect : Chip Select high */
    LCD_CS_HIGH();
}

/**
  * @brief  Read register value.
  * @param  Reg
  * @retval None
  */
uint16_t LCD_IO_ReadData(uint16_t Reg)
{
    uint32_t readvalue = 0;

    /* Send Reg value to Read */
    LCD_IO_WriteReg(Reg);

    /* Reset LCD control line(/CS) and Send command */
    LCD_CS_LOW();

    /* Send Start Byte */
    SPIx_Write(START_BYTE | LCD_READ_REG);
    /* Read Upper Byte */
    SPIx_Write(0xFF);
    readvalue = SPIx_Read();
    readvalue = readvalue << 8;
    readvalue |= SPIx_Read();

    HAL_Delay(10);

    /* Deselect : Chip Select high */
    LCD_CS_HIGH();
    return readvalue;
}

/**
  * @brief  Wait for loop in ms.
  * @param  Delay in ms.
  * @retval None
  */
void LCD_Delay(uint32_t Delay)
{
    HAL_Delay(Delay);
}

/******************************** LINK SD Card ********************************/

/**
  * @brief  Initializes the SD Card and put it into StandBy State (Ready for 
  *         data transfer).
  * @retval None
  */
void SD_IO_Init(void)
{
    GPIO_InitTypeDef gpioinitstruct;
    uint8_t counter;

    /* SD_CS_GPIO and SD_DETECT_GPIO Periph clock enable */
    SD_CS_GPIO_CLK_ENABLE();
    SD_DETECT_GPIO_CLK_ENABLE();

    /* Configure SD_CS_PIN pin: SD Card CS pin */
    gpioinitstruct.Pin = SD_CS_PIN;
    gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
    gpioinitstruct.Pull = GPIO_PULLUP;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SD_CS_GPIO_PORT, &gpioinitstruct);

    /* Configure SD_DETECT_PIN pin: SD Card detect pin */
    gpioinitstruct.Pin = SD_DETECT_PIN;
    gpioinitstruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    gpioinitstruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(SD_DETECT_GPIO_PORT, &gpioinitstruct);

    /* Enable and set SD EXTI Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(SD_DETECT_EXTI_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(SD_DETECT_EXTI_IRQn);

    /*------------Put SD in SPI mode--------------*/
    /* SD SPI Config */
    SPIx_Init();

    /* SD chip select high */
    SD_CS_HIGH();

    /* Send dummy byte 0xFF, 10 times with CS high */
    /* Rise CS and MOSI for 80 clocks cycles */
    for (counter = 0; counter <= 9; counter++)
    {
        /* Send dummy byte 0xFF */
        SD_IO_WriteByte(SD_DUMMY_BYTE);
    }
}

/**
  * @brief  Write a byte on the SD.
  * @param  Data: byte to send.
  * @retval None
  */
void SD_IO_WriteByte(uint8_t Data)
{
    /* Send the byte */
    SPIx_Write(Data);
}

/**
  * @brief  Read a byte from the SD.
  * @retval The received byte.
  */
uint8_t SD_IO_ReadByte(void)
{
    uint8_t data = 0;

    /* Get the received data */
    data = SPIx_Read();

    /* Return the shifted data */
    return data;
}

/**
  * @brief  Send 5 bytes command to the SD card and get response
  * @param  Cmd: The user expected command to send to SD card.
  * @param  Arg: The command argument.
  * @param  Crc: The CRC.
  * @param  Response: Expected response from the SD card
  * @retval  HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef SD_IO_WriteCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc, uint8_t Response)
{
    uint32_t counter = 0x00;
    uint8_t frame[6];

    /* Prepare Frame to send */
    frame[0] = (Cmd | 0x40);         /* Construct byte 1 */
    frame[1] = (uint8_t)(Arg >> 24); /* Construct byte 2 */
    frame[2] = (uint8_t)(Arg >> 16); /* Construct byte 3 */
    frame[3] = (uint8_t)(Arg >> 8);  /* Construct byte 4 */
    frame[4] = (uint8_t)(Arg);       /* Construct byte 5 */
    frame[5] = (Crc);                /* Construct CRC: byte 6 */

    /* SD chip select low */
    SD_CS_LOW();

    /* Send Frame */
    for (counter = 0; counter < 6; counter++)
    {
        SD_IO_WriteByte(frame[counter]); /* Send the Cmd bytes */
    }

    if (Response != SD_NO_RESPONSE_EXPECTED)
    {
        return SD_IO_WaitResponse(Response);
    }

    return HAL_OK;
}

/**
  * @brief  Wait response from the SD card
  * @param  Response: Expected response from the SD card
  * @retval  HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef SD_IO_WaitResponse(uint8_t Response)
{
    uint32_t timeout = 0xFFFF;
    uint8_t resp = 0;
    /* Check if response is got or a timeout is happen */
    resp = SD_IO_ReadByte();
    while ((resp != Response) && timeout)
    {
        timeout--;
        resp = SD_IO_ReadByte();
    }

    if (timeout == 0)
    {
        /* After time out */
        return HAL_TIMEOUT;
    }
    else
    {
        /* Right response got */
        return HAL_OK;
    }
}

/**
  * @brief  Send dummy byte with CS High
  * @retval None
  */
void SD_IO_WriteDummy(void)
{
    /* SD chip select high */
    SD_CS_HIGH();

    /* Send Dummy byte 0xFF */
    SD_IO_WriteByte(SD_DUMMY_BYTE);
}

#endif /* HAL_SPI_MODULE_ENABLED */

#ifdef HAL_I2C_MODULE_ENABLED
/********************************* LINK I2C EEPROM *****************************/
/**
  * @brief  Initializes peripherals used by the I2C EEPROM driver.
  * @retval None
  */
void EEPROM_I2C_IO_Init(void)
{
    I2Cx_Init();
}

/**
  * @brief  Write data to I2C EEPROM driver
  * @param  DevAddress: Target device address
  * @param  MemAddress: Internal memory address
  * @param  pBuffer: Pointer to data buffer
  * @param  BufferSize: Amount of data to be sent
  * @retval HAL status
  */
HAL_StatusTypeDef EEPROM_I2C_IO_WriteData(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pBuffer, uint32_t BufferSize)
{
    return (I2Cx_WriteBuffer(DevAddress, MemAddress, I2C_MEMADD_SIZE_16BIT, pBuffer, BufferSize));
}

/**
  * @brief  Read data from I2C EEPROM driver
  * @param  DevAddress: Target device address
  * @param  MemAddress: Internal memory address
  * @param  pBuffer: Pointer to data buffer
  * @param  BufferSize: Amount of data to be read
  * @retval HAL status
  */
HAL_StatusTypeDef EEPROM_I2C_IO_ReadData(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pBuffer, uint32_t BufferSize)
{
    return (I2Cx_ReadBuffer(DevAddress, MemAddress, I2C_MEMADD_SIZE_16BIT, pBuffer, BufferSize));
}

/**
* @brief  Checks if target device is ready for communication. 
* @note   This function is used with Memory devices
* @param  DevAddress: Target device address
* @param  Trials: Number of trials
* @retval HAL status
*/
HAL_StatusTypeDef EEPROM_I2C_IO_IsDeviceReady(uint16_t DevAddress, uint32_t Trials)
{
    return (I2Cx_IsDeviceReady(DevAddress, Trials));
}

/********************************* LINK I2C TEMPERATURE SENSOR *****************************/
/**
  * @brief  Initializes peripherals used by the I2C Temperature Sensor driver.
  * @retval None
  */
void TSENSOR_IO_Init(void)
{
    I2Cx_Init();
}

/**
  * @brief  Writes one byte to the TSENSOR.
  * @param  DevAddress: Target device address
  * @param  pBuffer: Pointer to data buffer
  * @param  WriteAddr: TSENSOR's internal address to write to.
  * @param  Length: Number of data to write
  * @retval None
  */
void TSENSOR_IO_Write(uint16_t DevAddress, uint8_t *pBuffer, uint8_t WriteAddr, uint16_t Length)
{
    I2Cx_WriteBuffer(DevAddress, WriteAddr, I2C_MEMADD_SIZE_8BIT, pBuffer, Length);
}

/**
  * @brief  Reads one byte from the TSENSOR.
  * @param  DevAddress: Target device address
  * @param  pBuffer : pointer to the buffer that receives the data read from the TSENSOR.
  * @param  ReadAddr : TSENSOR's internal address to read from.
  * @param  Length: Number of data to read
  * @retval None
  */
void TSENSOR_IO_Read(uint16_t DevAddress, uint8_t *pBuffer, uint8_t ReadAddr, uint16_t Length)
{
    I2Cx_ReadBuffer(DevAddress, ReadAddr, I2C_MEMADD_SIZE_8BIT, pBuffer, Length);
}

/**
* @brief  Checks if Temperature Sensor is ready for communication. 
  * @param  DevAddress: Target device address
  * @param  Trials: Number of trials
* @retval HAL status
*/
uint16_t TSENSOR_IO_IsDeviceReady(uint16_t DevAddress, uint32_t Trials)
{
    return (I2Cx_IsDeviceReady(DevAddress, Trials));
}

/***************************** LINK ACCELERO *****************************/
/**
  * @brief  Configures ACCELEROMETER SPI interface.
  * @retval None
  */
void ACCELERO_IO_Init(void)
{
    /* Initialize the IO functionalities */
    BSP_IO_Init();
}

/**
  * @brief     Configures ACCELERO INT2 config.
                   EXTI0 is already used by user button so INT1 is configured here
  * @retval   None
  */
void ACCELERO_IO_ITConfig(void)
{
    BSP_IO_ConfigPin(MEMS_ALL_PINS, IO_MODE_IT_FALLING_EDGE);
}

/**
  * @brief  Writes one byte to the ACCELEROMETER.
  * @param  pBuffer : pointer to the buffer  containing the data to be written to the ACCELEROMETER.
  * @param  WriteAddr : ACCELEROMETER's internal address to write to.
  * @param  NumByteToWrite: Number of bytes to write.
  * @retval None
  */
void ACCELERO_IO_Write(uint8_t *pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite)
{
    I2Cx_WriteBuffer(L1S302DL_I2C_ADDRESS, WriteAddr, I2C_MEMADD_SIZE_8BIT, pBuffer, NumByteToWrite);
}

/**
  * @brief  Reads a block of data from the ACCELEROMETER.
  * @param  pBuffer : pointer to the buffer that receives the data read from the ACCELEROMETER.
  * @param  ReadAddr : ACCELEROMETER's internal address to read from.
  * @param  NumByteToRead : number of bytes to read from the ACCELEROMETER.
  * @retval None
  */
void ACCELERO_IO_Read(uint8_t *pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{
    I2Cx_ReadBuffer(L1S302DL_I2C_ADDRESS, ReadAddr, I2C_MEMADD_SIZE_8BIT, pBuffer, NumByteToRead);
}

/********************************* LINK AUDIO ***********************************/

/**
  * @brief  Initializes Audio low level.
  * @retval None
  */
void AUDIO_IO_Init(void)
{
    /* Initialize the IO functionalities */
    BSP_IO_Init();

    BSP_IO_ConfigPin(AUDIO_RESET_PIN, IO_MODE_OUTPUT);

    /* Power Down the codec */
    BSP_IO_WritePin(AUDIO_RESET_PIN, GPIO_PIN_RESET);

    /* wait for a delay to insure registers erasing */
    HAL_Delay(5);

    /* Power on the codec */
    BSP_IO_WritePin(AUDIO_RESET_PIN, GPIO_PIN_SET);

    /* wait for a delay to insure registers erasing */
    HAL_Delay(5);
}

/**
  * @brief  DeInitializes Audio low level.
  * @note   This function is intentionally kept empty, user should define it.
  */
void AUDIO_IO_DeInit(void)
{
}

/**
  * @brief  Writes a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @param  Value: Data to be written
  * @retval None
  */
void AUDIO_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
    I2Cx_WriteData(Addr, Reg, Value);
}

/**
  * @brief  Reads a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @retval Data to be read
  */
uint8_t AUDIO_IO_Read(uint8_t Addr, uint8_t Reg)
{
    return I2Cx_ReadData(Addr, Reg);
}

#endif /* HAL_I2C_MODULE_ENABLED */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
