/**
  ******************************************************************************
  * @file    stm3210c_eval.h
  * @author  MCD Application Team
  * @version V6.0.2
  * @date    29-April-2016
  * @brief   This file contains definitions for STM3210C_EVAL's LEDs, 
  *          push-buttons and COM ports hardware resources.
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

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup STM3210C_LORATRACKER
  * @{
  */ 

/** @addtogroup STM3210C_LORATRACKER_COMMON
  * @{
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM3210C_LORATRACKER_H
#define __STM3210C_LORATRACKER_H

#ifdef __cplusplus
 extern "C" {
#endif 




/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"



/** @defgroup STM3210C_EVAL_Exported_Types Exported Types
  * @{
  */

/**
 * @brief LED Types Definition
 */

#if defined(BSP_V500_TEST)
typedef enum 
{
  LED1 = 0,
  LED2 = 1,
  LED3 = 2, 

  LED_GREEN  = LED1,
  LED_ORANGE = LED2,
  LED_RED    = LED3,
} Led_TypeDef;
#else
typedef enum 
{
  LED1 = 0,
  LED2 = 1,

  LED_GREEN  = LED1,
  LED_RED    = LED2,

} Led_TypeDef;
#endif

#if defined(BSP_V500_TEST)
typedef enum 
{
  OUTPUT_WDI           = 0,
  OUTPUT_PS_HOLD       = 1,
  OUTPUT_WDE           = 2,
  OUTPUT_GPS_POWER     = 3,
  OUTPUT_LORA_POWER    = 4,
  OUTPUT_LORA_RESET    = 5,
} Output_TypeDef;
#else
typedef enum 
{
  OUTPUT_WKUP = 0,
  OUTPUT_NRST = 1,
  OUTPUT_PRST = 2,
} Output_TypeDef;
#endif


/**
 * @brief BUTTON Types Definition
 */
typedef enum 
{
  INPUT_FACTORY = 0,
} Input_TypeDef;

typedef enum 
{  
  INPUT_MODE_GPIO = 0,
  INPUT_MODE_EXTI = 1
} InputMode_TypeDef;

#if 0
/**
 * @brief BUTTON Types Definition
 */
typedef enum 
{
  BUTTON_WAKEUP = 0,
  BUTTON_TAMPER = 1,
  BUTTON_KEY    = 2,

} Button_TypeDef;

typedef enum 
{  
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
} ButtonMode_TypeDef;

/**
 * @brief JOYSTICK Types Definition
 */
typedef enum 
{ 
  JOY_SEL   = 0,
  JOY_LEFT  = 1,
  JOY_RIGHT = 2,
  JOY_DOWN  = 3,
  JOY_UP    = 4,
  JOY_NONE  = 5

}JOYState_TypeDef;

typedef enum 
{ 
  JOY_MODE_GPIO = 0,
  JOY_MODE_EXTI = 1

}JOYMode_TypeDef;

#endif

/**
 * @brief COM Types Definition
 */
typedef enum 
{
  COM1 = 0,
  COM2 = 1
} COM_TypeDef;
/**
  * @}
  */ 

/** @defgroup STM3210C_EVAL_Exported_Constants Exported Constants
  * @{
  */ 

/** 
  * @brief  Define for STM3210C_EVAL board  
  */ 
#if !defined (USE_STM3210C_EVAL)
 #define USE_STM3210C_EVAL
#endif
  
/** @addtogroup STM3210C_EVAL_LED
  * @{
  */

#if defined(BSP_V500_TEST)
#define LEDn                             3

#define LED1_PIN                         GPIO_PIN_2             /* yellow*/
#define LED1_GPIO_PORT                   GPIOD
#define LED1_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOD_CLK_ENABLE()
#define LED1_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOD_CLK_DISABLE()
  
#define LED2_PIN                         GPIO_PIN_10             /* blue*/
#define LED2_GPIO_PORT                   GPIOC
#define LED2_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOC_CLK_ENABLE()
#define LED2_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOC_CLK_DISABLE()


#define LED3_PIN                         GPIO_PIN_11            /* red*/
#define LED3_GPIO_PORT                   GPIOC
#define LED3_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOC_CLK_ENABLE()
#define LED3_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOC_CLK_DISABLE()


#define LEDx_GPIO_CLK_ENABLE(__LED__)    do { if ((__LED__) == LED1) LED1_GPIO_CLK_ENABLE(); else \
                                              if ((__LED__) == LED2) LED2_GPIO_CLK_ENABLE(); else \
                                              if ((__LED__) == LED3) LED3_GPIO_CLK_ENABLE(); } while(0)

#define LEDx_GPIO_CLK_DISABLE(__LED__)   (((__LED__) == LED1) ? LED1_GPIO_CLK_DISABLE() :\
                                          ((__LED__) == LED2) ? LED2_GPIO_CLK_DISABLE() :\
                                          ((__LED__) == LED3) ? LED3_GPIO_CLK_DISABLE() : 0 )

#else

#define LEDn                             2

#define LED1_PIN                         GPIO_PIN_8             /* green*/
#define LED1_GPIO_PORT                   GPIOC
#define LED1_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOD_CLK_ENABLE()
#define LED1_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOD_CLK_DISABLE()
  
#define LED2_PIN                         GPIO_PIN_9             /* red*/
#define LED2_GPIO_PORT                   GPIOC
#define LED2_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOC_CLK_ENABLE()
#define LED2_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOC_CLK_DISABLE()



#define LEDx_GPIO_CLK_ENABLE(__LED__)    do { if ((__LED__) == LED1) LED1_GPIO_CLK_ENABLE(); else \
                                              if ((__LED__) == LED2) LED2_GPIO_CLK_ENABLE(); } while(0)

#define LEDx_GPIO_CLK_DISABLE(__LED__)   (((__LED__) == LED1) ? LED1_GPIO_CLK_DISABLE() :\
                                          ((__LED__) == LED2) ? LED2_GPIO_CLK_DISABLE() : 0 )
#endif


#define TIMx                           TIM3
#define TIMx_CLK_ENABLE()              __HAL_RCC_TIM3_CLK_ENABLE()


/* Definition for TIMx's NVIC */
#define TIMx_IRQn                      TIM3_IRQn
#define TIMx_IRQHandler                TIM3_IRQHandler
/**
  * @}
  */


/**
 * @brief output gpio
 */

#if defined(BSP_V500_TEST)

#define OUTGPIOn                          6

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


#define OUTPUTx_GPIO_CLK_ENABLE(__OUTPUTGPIO__) do { if ((__OUTPUTGPIO__) == OUTPUT_WDI) WDI_HOLD_GPIO_CLK_ENABLE()  ; else \
                                                 if ((__OUTPUTGPIO__) == OUTPUT_PS_HOLD) PS_HOLD_GPIO_CLK_ENABLE()  ; else \
                                                 if ((__OUTPUTGPIO__) == OUTPUT_WDE) WDE_GPIO_CLK_ENABLE()  ; else \
                                                 if ((__OUTPUTGPIO__) == OUTPUT_GPS_POWER) GPS_POWER_EN_CLK_ENABLE()  ; else \
                                                 if ((__OUTPUTGPIO__) == OUTPUT_LORA_POWER) LORA_POWER_EN_CLK_ENABLE()  ; else \
                                                 if ((__OUTPUTGPIO__) == OUTPUT_LORA_RESET) LORA_RESET_EN_CLK_ENABLE();} while(0)

#define OUTPUTx_GPIO_CLK_DISABLE(__OUTPUTGPIO__)    (((__OUTPUTGPIO__) == OUTPUT_WDI) WDI_HOLD_GPIO_CLK_DISBLE()  :\
                                                 ((__OUTPUTGPIO__) == OUTPUT_PS_HOLD) PS_HOLD_GPIO_CLK_DISABLE()  :\
                                                 ((__OUTPUTGPIO__) == OUTPUT_WDE) WDE_GPIO_CLK_DISABLE()  :\
                                                 ((__OUTPUTGPIO__) == OUTPUT_GPS_POWER) GPS_POWER_EN_CLK_DISABLE()  :\
                                                 ((__OUTPUTGPIO__) == OUTPUT_LORA_POWER) LORA_POWER_EN_CLK_DISABLE()  :\
                                                 ((__OUTPUTGPIO__) == OUTPUT_LORA_RESET) LORA_RESET_EN_CLK_DISABLE()  : 0 )
#else
#define OUTGPIOn                          3

#define WKUP_PIN                          GPIO_PIN_0
#define WKUP_GPIO_PORT		                GPIOA
#define WKUP_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOA_CLK_ENABLE()
#define WKUP_GPIO_CLK_DISBLE()            __HAL_RCC_GPIOA_CLK_DISABLE()

#define NRST_PIN                    	    GPIO_PIN_1
#define NRST_GPIO_PORT              	    GPIOA
#define NRST_GPIO_CLK_ENABLE()    		    __HAL_RCC_GPIOA_CLK_ENABLE()
#define NRST_GPIO_CLK_DISABLE()    	      __HAL_RCC_GPIOA_CLK_DISABLE()

#define PRST_PIN                    			GPIO_PIN_0
#define PRST_GPIO_PORT              			GPIOB
#define PRST_GPIO_CLK_ENABLE()    			  __HAL_RCC_GPIOB_CLK_ENABLE()
#define PRST_GPIO_CLK_DISABLE()    		    __HAL_RCC_GPIOB_CLK_DISABLE()

#define OUTPUTx_GPIO_CLK_ENABLE(__OUTPUTGPIO__) do { if ((__OUTPUTGPIO__) == OUTPUT_WKUP) WKUP_GPIO_CLK_ENABLE()  ; else \
                                                 if ((__OUTPUTGPIO__) == OUTPUT_NRST) NRST_GPIO_CLK_ENABLE()  ; else \
                                                 if ((__OUTPUTGPIO__) == OUTPUT_PRST) PRST_GPIO_CLK_ENABLE();} while(0)

#define OUTPUTx_GPIO_CLK_DISABLE(__OUTPUTGPIO__)    (((__OUTPUTGPIO__) == OUTPUT_WKUP) WKUP_GPIO_CLK_DISBLE()  :\
                                                 ((__OUTPUTGPIO__) == OUTPUT_NRST) NRST_GPIO_CLK_DISABLE()  :\
                                                 ((__OUTPUTGPIO__) == OUTPUT_PRST) PRST_GPIO_CLK_DISABLE()  : 0 )

/** @addtogroup Lora_EVAL_input
  * @{
  */  

#define INPUTn                          1

/**
 * @brief input gpio
 */


#define FACTORY_INPUT_PIN                      GPIO_PIN_2             /* PB.02*/
#define FACTORY_INPUT_GPIO_PORT                GPIOB
#define FACTORY_INPUT_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOB_CLK_ENABLE()
#define FACTORY_INPUT_GPIO_CLK_DISABLE()       __HAL_RCC_GPIOB_CLK_DISABLE()
#define FACTORY_INPUT_EXTI_IRQn                EXTI9_5_IRQn


#define INPUTx_GPIO_CLK_ENABLE(__INPUT__)       do{if((__INPUT__) == INPUT_FACTORY) FACTORY_INPUT_GPIO_CLK_ENABLE(); else 0; } while(0);
#define INPUTx_GPIO_CLK_DISABLE(__INPUT__)    (((__INPUT__) == INPUT_FACTORY) FACTORY_INPUT_GPIO_CLK_DISABLE();  : 0 )

#endif

#if 0

/** @addtogroup STM3210C_EVAL_BUTTON
  * @{
  */  
#define BUTTONn                          3

/**
 * @brief Tamper push-button
 */
#define TAMPER_BUTTON_PIN                   GPIO_PIN_13             /* PC.13*/
#define TAMPER_BUTTON_GPIO_PORT             GPIOC
#define TAMPER_BUTTON_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()
#define TAMPER_BUTTON_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOC_CLK_DISABLE()
#define TAMPER_BUTTON_EXTI_IRQn             EXTI15_10_IRQn

/**
 * @brief Key push-button
 */
#define KEY_BUTTON_PIN                      GPIO_PIN_9             /* PB.09*/
#define KEY_BUTTON_GPIO_PORT                GPIOB
#define KEY_BUTTON_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOB_CLK_ENABLE()
#define KEY_BUTTON_GPIO_CLK_DISABLE()       __HAL_RCC_GPIOB_CLK_DISABLE()
#define KEY_BUTTON_EXTI_IRQn                EXTI9_5_IRQn

/**
 * @brief Wake-up push-button
 */
#define WAKEUP_BUTTON_PIN                   GPIO_PIN_0             /* PA.00*/
#define WAKEUP_BUTTON_GPIO_PORT             GPIOA
#define WAKEUP_BUTTON_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define WAKEUP_BUTTON_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOA_CLK_DISABLE()
#define WAKEUP_BUTTON_EXTI_IRQn             EXTI0_IRQn

#define BUTTONx_GPIO_CLK_ENABLE(__BUTTON__) do { if ((__BUTTON__) == BUTTON_TAMPER) TAMPER_BUTTON_GPIO_CLK_ENABLE()  ; else \
                                                 if ((__BUTTON__) == BUTTON_KEY) KEY_BUTTON_GPIO_CLK_ENABLE()  ; else \
                                                 if ((__BUTTON__) == BUTTON_WAKEUP) WAKEUP_BUTTON_GPIO_CLK_ENABLE();} while(0)

#define BUTTONx_GPIO_CLK_DISABLE(__BUTTON__)    (((__BUTTON__) == BUTTON_TAMPER) TAMPER_BUTTON_GPIO_CLK_DISABLE()  :\
                                                 ((__BUTTON__) == BUTTON_KEY) KEY_BUTTON_GPIO_CLK_DISABLE()  :\
                                                 ((__BUTTON__) == BUTTON_WAKEUP) WAKEUP_BUTTON_GPIO_CLK_DISABLE()  : 0 )

/**
  * @brief IO Pins definition 
  */ 
/* Joystick */
#define JOY_SEL_PIN                  (IO2_PIN_7) /* IO_Expander_2 */
#define JOY_DOWN_PIN                 (IO2_PIN_6) /* IO_Expander_2 */
#define JOY_LEFT_PIN                 (IO2_PIN_5) /* IO_Expander_2 */
#define JOY_RIGHT_PIN                (IO2_PIN_4) /* IO_Expander_2 */
#define JOY_UP_PIN                   (IO2_PIN_3) /* IO_Expander_2 */
#define JOY_NONE_PIN                 JOY_ALL_PINS
#define JOY_ALL_PINS                 (JOY_SEL_PIN | JOY_DOWN_PIN | JOY_LEFT_PIN | JOY_RIGHT_PIN | JOY_UP_PIN)

/* MEMS */
#define MEMS_INT1_PIN                (IO1_PIN_3) /* IO_Expander_1 */ /* Input */
#define MEMS_INT2_PIN                (IO1_PIN_2) /* IO_Expander_1 */ /* Input */
#define MEMS_ALL_PINS                (MEMS_INT1_PIN | MEMS_INT2_PIN)

#define AUDIO_RESET_PIN              (IO2_PIN_2) /* IO_Expander_2 */ /* Output */
#define MII_INT_PIN                  (IO2_PIN_0) /* IO_Expander_2 */ /* Output */
#define VBAT_DIV_PIN                 (IO1_PIN_0) /* IO_Expander_1 */ /* Output */

#endif
/**
  * @}
  */ 

/** @addtogroup STM3210C_EVAL_COM
  * @{
  */
#define COMn                             1

/**
 * @brief Definition for COM port1, connected to USART2
 */ 
#define EVAL_COM1                        USART1
#define EVAL_COM1_CLK_ENABLE()           __HAL_RCC_USART1_CLK_ENABLE()
#define EVAL_COM1_CLK_DISABLE()          __HAL_RCC_USART1_CLK_DISABLE()

#define AFIOCOM1_CLK_ENABLE()            __HAL_RCC_AFIO_CLK_ENABLE()
#define AFIOCOM1_CLK_DISABLE()           __HAL_RCC_AFIO_CLK_DISABLE()

#define EVAL_COM1_TX_PIN                 GPIO_PIN_9             /* PD.05*/
#define EVAL_COM1_TX_GPIO_PORT           GPIOA
#define EVAL_COM1_TX_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define EVAL_COM1_TX_GPIO_CLK_DISABLE()  __HAL_RCC_GPIOA_CLK_DISABLE()

#define EVAL_COM1_RX_PIN                 GPIO_PIN_10             /* PD.06*/
#define EVAL_COM1_RX_GPIO_PORT           GPIOA
#define EVAL_COM1_RX_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define EVAL_COM1_RX_GPIO_CLK_DISABLE()  __HAL_RCC_GPIOA_CLK_DISABLE()

#define EVAL_COM1_IRQn                   USART2_IRQn

#define COMx_CLK_ENABLE(__INDEX__)              do { if((__INDEX__) == COM1) EVAL_COM1_CLK_ENABLE();} while(0)
#define COMx_CLK_DISABLE(__INDEX__)             (((__INDEX__) == COM1) ? EVAL_COM1_CLK_DISABLE() : 0)

#define AFIOCOMx_CLK_ENABLE(__INDEX__)          do { if((__INDEX__) == COM1) AFIOCOM1_CLK_ENABLE();} while(0)
#define AFIOCOMx_CLK_DISABLE(__INDEX__)         (((__INDEX__) == COM1) ? AFIOCOM1_CLK_DISABLE() : 0)

#define AFIOCOMx_REMAP(__INDEX__)               (((__INDEX__) == COM1) ? (AFIO->MAPR |= (AFIO_MAPR_USART2_REMAP)) : 0)

#define COMx_TX_GPIO_CLK_ENABLE(__INDEX__)      do { if((__INDEX__) == COM1) EVAL_COM1_TX_GPIO_CLK_ENABLE();} while(0)
#define COMx_TX_GPIO_CLK_DISABLE(__INDEX__)     (((__INDEX__) == COM1) ? EVAL_COM1_TX_GPIO_CLK_DISABLE() : 0)

#define COMx_RX_GPIO_CLK_ENABLE(__INDEX__)      do { if((__INDEX__) == COM1) EVAL_COM1_RX_GPIO_CLK_ENABLE();} while(0)
#define COMx_RX_GPIO_CLK_DISABLE(__INDEX__)     (((__INDEX__) == COM1) ? EVAL_COM1_RX_GPIO_CLK_DISABLE() : 0)

/* ########################## hw definition ############################## */


#define LOG_USARTx                           USART1
#define LOG_USARTx_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE();
#define LOG_USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define LOG_USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define LOG_USARTx_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define LOG_USARTx_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for USARTx Pins */
#define LOG_USARTx_TX_PIN                    GPIO_PIN_9
#define LOG_USARTx_TX_GPIO_PORT              GPIOA
#define LOG_USARTx_RX_PIN                    GPIO_PIN_10
#define LOG_USARTx_RX_GPIO_PORT              GPIOA


/* Definition for USARTx's DMA */
#define LOG_USARTx_TX_DMA_CHANNEL             DMA1_Channel4
#define LOG_USARTx_RX_DMA_CHANNEL             DMA1_Channel5

/* Definition for USARTx's NVIC */
#define LOG_USARTx_DMA_TX_IRQn                DMA1_Channel4_IRQn
#define LOG_USARTx_DMA_RX_IRQn                DMA1_Channel5_IRQn
#define LOG_USARTx_DMA_TX_IRQHandler          DMA1_Channel4_IRQHandler
#define LOG_USARTx_DMA_RX_IRQHandler          DMA1_Channel5_IRQHandler

/* Definition for USARTx's NVIC */
#define LOG_USARTx_IRQn                      USART1_IRQn
#define LOG_USARTx_IRQHandler                USART1_IRQHandler


/* Definition for USART2 clock resources */
#define LOR_USARTx                           USART2
#define LOR_USARTx_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE();
#define LOR_DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define LOR_USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define LOR_USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()

#define LOR_USARTx_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define LOR_USARTx_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()

/* Definition for USARTx Pins */
#define LOR_USARTx_TX_PIN                    GPIO_PIN_2
#define LOR_USARTx_TX_GPIO_PORT              GPIOA
#define LOR_USARTx_RX_PIN                    GPIO_PIN_3
#define LOR_USARTx_RX_GPIO_PORT              GPIOA

/* Definition for USARTx's DMA */
#define LOR_USARTx_TX_DMA_CHANNEL             DMA1_Channel7
#define LOR_USARTx_RX_DMA_CHANNEL             DMA1_Channel6

/* Definition for USARTx's NVIC */
#define LOR_USARTx_DMA_TX_IRQn                DMA1_Channel7_IRQn
#define LOR_USARTx_DMA_RX_IRQn                DMA1_Channel6_IRQn
#define LOR_USARTx_DMA_TX_IRQHandler          DMA1_Channel7_IRQHandler
#define LOR_USARTx_DMA_RX_IRQHandler          DMA1_Channel6_IRQHandler

/* Definition for USARTx's NVIC */
#define LOR_USARTx_IRQn                      USART2_IRQn
#define LOR_USARTx_IRQHandler                USART2_IRQHandler

/* Size of Trasmission buffer */
#define LOR_TXBUFFERSIZE                      (COUNTOF(lorTxBuffer) - 1)
/* Size of Reception buffer */
#define LOR_RXBUFFERSIZE                      10


  /* Definition for USART3 clock resources */
#define EXT_USARTx                           USART3
#define EXT_USARTx_CLK_ENABLE()              __HAL_RCC_USART3_CLK_ENABLE();
#define EXT_DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define EXT_USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define EXT_USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
  
#define EXT_USARTx_FORCE_RESET()             __HAL_RCC_USART3_FORCE_RESET()
#define EXT_USARTx_RELEASE_RESET()           __HAL_RCC_USART3_RELEASE_RESET()
  
  /* Definition for USARTx Pins */
#define EXT_USARTx_TX_PIN                    GPIO_PIN_10
#define EXT_USARTx_TX_GPIO_PORT              GPIOB
#define EXT_USARTx_RX_PIN                    GPIO_PIN_11
#define EXT_USARTx_RX_GPIO_PORT              GPIOB
  
  /* Definition for USARTx's DMA */
#define EXT_USARTx_TX_DMA_CHANNEL             DMA1_Channel2
#define EXT_USARTx_RX_DMA_CHANNEL             DMA1_Channel3
  
  /* Definition for USARTx's NVIC */
#define EXT_USARTx_DMA_TX_IRQn                DMA1_Channel2_IRQn
#define EXT_USARTx_DMA_RX_IRQn                DMA1_Channel3_IRQn
#define EXT_USARTx_DMA_TX_IRQHandler          DMA1_Channel2_IRQHandler
#define EXT_USARTx_DMA_RX_IRQHandler          DMA1_Channel3_IRQHandler
  
  /* Definition for USARTx's NVIC */
#define EXT_USARTx_IRQn                      USART3_IRQn
#define EXT_USARTx_IRQHandler                USART3_IRQHandler
  
  /* Size of Trasmission buffer */
#define EXT_TXBUFFERSIZE                      (COUNTOF(extTxBuffer) - 1)
  /* Size of Reception buffer */
#define EXT_RXBUFFERSIZE                      10


  /* Definition for UART4 clock resources  */
  // GPS_USARTx is using UART4
#define GPS_USARTx                           UART4
#define GPS_USARTx_CLK_ENABLE()              __HAL_RCC_UART4_CLK_ENABLE();
#define GPS_DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define GPS_USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define GPS_USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
  
#define GPS_USARTx_FORCE_RESET()             __HAL_RCC_UART4_FORCE_RESET()
#define GPS_USARTx_RELEASE_RESET()           __HAL_RCC_UART4_RELEASE_RESET()
  
  /* Definition for USARTx Pins */
#define GPS_USARTx_TX_PIN                    GPIO_PIN_10
#define GPS_USARTx_TX_GPIO_PORT              GPIOC
#define GPS_USARTx_RX_PIN                    GPIO_PIN_11
#define GPS_USARTx_RX_GPIO_PORT              GPIOC

#if 0  
  /* Definition for USARTx's DMA */
#define GPS_USARTx_TX_DMA_CHANNEL             DMA1_Channel3
#define GPS_USARTx_RX_DMA_CHANNEL             DMA1_Channel2
  
  /* Definition for USARTx's NVIC */
#define GPS_USARTx_DMA_TX_IRQn                DMA1_Channel3_IRQn
#define GPS_USARTx_DMA_RX_IRQn                DMA1_Channel2_IRQn
#define GPS_USARTx_DMA_TX_IRQHandler          DMA1_Channel3_IRQHandler
#define GPS_USARTx_DMA_RX_IRQHandler          DMA1_Channel2_IRQHandler
#endif

  /* Definition for USARTx's NVIC */
#define GPS_USARTx_IRQn                      UART4_IRQn
#define GPS_USARTx_IRQHandler                UART4_IRQHandler
  
  /* Size of Trasmission buffer */
#define GPS_TXBUFFERSIZE                      (COUNTOF(gpsTxBuffer) - 1)
  /* Size of Reception buffer */
#define GPS_RXBUFFERSIZE                      10



/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* Exported functions ------------------------------------------------------- */

#if 0
/**
  * @}
  */ 

/** @addtogroup STM3210C_EVAL_BUS
  * @{
  */

/** 
  * @brief  IO Expander Interrupt line on EXTI  
  */ 
#define IOE_IT_PIN                       GPIO_PIN_14
#define IOE_IT_GPIO_PORT                 GPIOB
#define IOE_IT_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOB_CLK_ENABLE()
#define IOE_IT_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOB_CLK_DISABLE()
#define IOE_IT_EXTI_IRQn                 EXTI15_10_IRQn       
#define IOE_IT_EXTI_IRQHANDLER           EXTI15_10_IRQHandler

/* Exported constant IO ------------------------------------------------------*/
#define IO1_I2C_ADDRESS                       0x82
#define IO2_I2C_ADDRESS                       0x88
#define TS_I2C_ADDRESS                        0x82

/*The Slave ADdress (SAD) associated to the LIS302DL is 001110xb. SDO pad can be used 
to modify less significant bit of the device address. If SDO pad is connected to voltage 
supply LSb is �?�?(address 0011101b) else if SDO pad is connected to ground LSb value is 
�?�?(address 0011100b).*/
#define L1S302DL_I2C_ADDRESS                  0x38


/*##################### ACCELEROMETER ##########################*/
/* Read/Write command */
#define READWRITE_CMD                     ((uint8_t)0x80) 
/* Multiple byte read/write command */ 
#define MULTIPLEBYTE_CMD                  ((uint8_t)0x40)

/*##################### I2Cx ###################################*/
/* User can use this section to tailor I2Cx instance used and associated 
   resources */
/* Definition for I2Cx Pins */
#define EVAL_I2Cx_SCL_PIN                       GPIO_PIN_6        /* PB.06*/
#define EVAL_I2Cx_SCL_GPIO_PORT                 GPIOB
#define EVAL_I2Cx_SDA_PIN                       GPIO_PIN_7        /* PB.07*/
#define EVAL_I2Cx_SDA_GPIO_PORT                 GPIOB

/* Definition for I2Cx clock resources */
#define EVAL_I2Cx                               I2C1
#define EVAL_I2Cx_CLK_ENABLE()                  __HAL_RCC_I2C1_CLK_ENABLE()
#define EVAL_I2Cx_SDA_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOB_CLK_ENABLE()
#define EVAL_I2Cx_SCL_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOB_CLK_ENABLE() 

#define EVAL_I2Cx_FORCE_RESET()                 __HAL_RCC_I2C1_FORCE_RESET()
#define EVAL_I2Cx_RELEASE_RESET()               __HAL_RCC_I2C1_RELEASE_RESET()
    
/* Definition for I2Cx's NVIC */
#define EVAL_I2Cx_EV_IRQn                       I2C1_EV_IRQn
#define EVAL_I2Cx_EV_IRQHandler                 I2C1_EV_IRQHandler
#define EVAL_I2Cx_ER_IRQn                       I2C1_ER_IRQn
#define EVAL_I2Cx_ER_IRQHandler                 I2C1_ER_IRQHandler

/* I2C clock speed configuration (in Hz) */
#ifndef BSP_I2C_SPEED
 #define BSP_I2C_SPEED                            400000
#endif /* I2C_SPEED */


/* Maximum Timeout values for flags waiting loops. These timeouts are not based
   on accurate values, they just guarantee that the application will not remain
   stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define EVAL_I2Cx_TIMEOUT_MAX                   3000

/*##################### SPI3 ###################################*/
#define EVAL_SPIx                               SPI3
#define EVAL_SPIx_CLK_ENABLE()                  __HAL_RCC_SPI3_CLK_ENABLE()

#define EVAL_SPIx_SCK_GPIO_PORT                 GPIOC             /* PC.10*/
#define EVAL_SPIx_SCK_PIN                       GPIO_PIN_10
#define EVAL_SPIx_SCK_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOC_CLK_ENABLE()
#define EVAL_SPIx_SCK_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOC_CLK_DISABLE()

#define EVAL_SPIx_MISO_MOSI_GPIO_PORT           GPIOC
#define EVAL_SPIx_MISO_MOSI_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOC_CLK_ENABLE()
#define EVAL_SPIx_MISO_MOSI_GPIO_CLK_DISABLE()  __HAL_RCC_GPIOC_CLK_DISABLE()
#define EVAL_SPIx_MISO_PIN                      GPIO_PIN_11       /* PC.11*/
#define EVAL_SPIx_MOSI_PIN                      GPIO_PIN_12       /* PC.12*/
/* Maximum Timeout values for flags waiting loops. These timeouts are not based
   on accurate values, they just guarantee that the application will not remain
   stuck if the SPI communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define EVAL_SPIx_TIMEOUT_MAX                   1000

/**
  * @}
  */ 

/** @addtogroup STM3210C_EVAL_COMPONENT
  * @{
  */

/*##################### LCD ###################################*/
/* Chip Select macro definition */
#define LCD_CS_LOW()       HAL_GPIO_WritePin(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, GPIO_PIN_RESET)
#define LCD_CS_HIGH()      HAL_GPIO_WritePin(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, GPIO_PIN_SET)

/** 
  * @brief  LCD Control Interface pins 
  */ 
#define LCD_NCS_PIN                             GPIO_PIN_2        /* PB.02*/
#define LCD_NCS_GPIO_PORT                       GPIOB
#define LCD_NCS_GPIO_CLK_ENABLE()               __HAL_RCC_GPIOB_CLK_ENABLE()
#define LCD_NCS_GPIO_CLK_DISABLE()              __HAL_RCC_GPIOB_CLK_DISABLE()

/*##################### SD ###################################*/
/* Chip Select macro definition */
#define SD_CS_LOW()       HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_PIN, GPIO_PIN_RESET)
#define SD_CS_HIGH()      HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_PIN, GPIO_PIN_SET)

/**
  * @brief  SD Control Interface pins
  */
#define SD_CS_PIN                               GPIO_PIN_4        /* PA.04*/
#define SD_CS_GPIO_PORT                         GPIOA
#define SD_CS_GPIO_CLK_ENABLE()                 __HAL_RCC_GPIOA_CLK_ENABLE()
#define SD_CS_GPIO_CLK_DISABLE()                __HAL_RCC_GPIOA_CLK_DISABLE()

/**
  * @brief  SD Detect Interface pins
  */
#define SD_DETECT_PIN                           GPIO_PIN_0
#define SD_DETECT_GPIO_PORT                     GPIOE
#define SD_DETECT_GPIO_CLK_ENABLE()             __HAL_RCC_GPIOE_CLK_ENABLE()
#define SD_DETECT_GPIO_CLK_DISABLE()            __HAL_RCC_GPIOE_CLK_DISABLE()
#define SD_DETECT_EXTI_IRQn                     EXTI0_IRQn

/*##################### AUDIO ##########################*/
/**
  * @brief  AUDIO I2C Interface pins
  */
#define AUDIO_I2C_ADDRESS                     0x94

#endif  
/**
  * @}
  */

/**
  * @}
  */ 



/** @addtogroup STM3210C_EVAL_Exported_Functions
  * @{
  */ 
char                    BSP_GetBootloaderVersion(void);
char                    BSP_GetAppVersion(void);
uint32_t                BSP_GetVersion(void);
void                    BSP_LED_Init(Led_TypeDef Led);
void                    BSP_LED_On(Led_TypeDef Led);
void                    BSP_LED_Off(Led_TypeDef Led);
void                    BSP_LED_Toggle(Led_TypeDef Led);

bool                    BSP_IsDownloadModeActivated(void);
void                    BSP_Download_Reset(void);
void                    BSP_Booting_Reset(void);

void                    BSP_HW_Reset(void);
void                    BSP_Lora_HW_Reset(void);
void                    BSP_Lora_Wakeup(void);
void                    BSP_Delay_HW_Reset(void);

void                    BSP_Input_Init(Input_TypeDef InputPin, InputMode_TypeDef Input_Mode);

#if 0
void                    BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode);
uint32_t                BSP_PB_GetState(Button_TypeDef Button);
#endif

#ifdef HAL_UART_MODULE_ENABLED
void                    BSP_COM_Init(COM_TypeDef COM, UART_HandleTypeDef* huart);
#endif /* HAL_UART_MODULE_ENABLED */
#ifdef HAL_I2C_MODULE_ENABLED
uint8_t                 BSP_JOY_Init(JOYMode_TypeDef Joy_Mode);
JOYState_TypeDef        BSP_JOY_GetState(void);
#endif /* HAL_I2C_MODULE_ENABLED */

void BSP_OUTGPIO_init(Output_TypeDef Outpin, GPIO_PinState initialstatus);
void BSP_OUTGPIO_Low(Output_TypeDef Outpin);
void BSP_OUTGPIO_High(Output_TypeDef Outpin);
/**
  * @}
  */


#ifdef __cplusplus
}
#endif
  
#endif /* __STM3210C_LORATRACKER_H */

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
