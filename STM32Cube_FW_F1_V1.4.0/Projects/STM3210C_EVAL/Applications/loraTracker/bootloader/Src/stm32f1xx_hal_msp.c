/**
  ******************************************************************************
  * @file    UART/UART_Printf/Src/stm32f1xx_hal_msp.c
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    29-April-2016
  * @brief   HAL MSP module.
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
#include "main.h"

/** @addtogroup STM32F1xx_HAL_Examples
  * @{
  */

/** @defgroup HAL_MSP
  * @brief HAL MSP module.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions
  * @{
  */

/**
  * @brief UART MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    static DMA_HandleTypeDef hdma_lor_tx;
    static DMA_HandleTypeDef hdma_lor_rx;
    static DMA_HandleTypeDef hdma_ext_tx;
    static DMA_HandleTypeDef hdma_ext_rx;
    static DMA_HandleTypeDef hdma_gps_tx;
    static DMA_HandleTypeDef hdma_gps_rx;

    if (0)//huart->Instance == LOG_USARTx)
    {

        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /* Enable GPIO TX/RX clock */
        LOG_USARTx_TX_GPIO_CLK_ENABLE();
        LOG_USARTx_RX_GPIO_CLK_ENABLE();

        /* Enable USARTx clock */
        LOG_USARTx_CLK_ENABLE();

        /*##-2- Configure peripheral GPIO ##########################################*/
        /* UART TX GPIO pin configuration  */
        GPIO_InitStruct.Pin = LOG_USARTx_TX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init(LOG_USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

        /* UART RX GPIO pin configuration  */
        GPIO_InitStruct.Pin = LOG_USARTx_RX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

        HAL_GPIO_Init(LOG_USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

#ifdef PLATFORM_USE_DMA
        /*##-3- Configure the DMA ##################################################*/
        /* Configure the DMA handler for Transmission process */
        hdma_lor_tx.Instance = LOR_USARTx_TX_DMA_CHANNEL;
        hdma_lor_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_lor_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_lor_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_lor_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_lor_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_lor_tx.Init.Mode = DMA_NORMAL;
        hdma_lor_tx.Init.Priority = DMA_PRIORITY_LOW;

        HAL_DMA_Init(&hdma_lor_tx);

        /* Associate the initialized DMA handle to the UART handle */
        __HAL_LINKDMA(huart, hdmatx, hdma_lor_tx);

        /* Configure the DMA handler for reception process */
        hdma_lor_rx.Instance = LOR_USARTx_RX_DMA_CHANNEL;
        hdma_lor_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_lor_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_lor_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_lor_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_lor_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_lor_rx.Init.Mode = DMA_NORMAL;
        hdma_lor_rx.Init.Priority = DMA_PRIORITY_HIGH;

        HAL_DMA_Init(&hdma_lor_rx);

        /* Associate the initialized DMA handle to the the UART handle */
        __HAL_LINKDMA(huart, hdmarx, hdma_lor_rx);

        /*##-4- Configure the NVIC for DMA #########################################*/
        /* NVIC configuration for DMA transfer complete interrupt (USART1_TX) */
        HAL_NVIC_SetPriority(LOR_USARTx_DMA_TX_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(LOR_USARTx_DMA_TX_IRQn);

        /* NVIC configuration for DMA transfer complete interrupt (USART1_RX) */
        HAL_NVIC_SetPriority(LOR_USARTx_DMA_RX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(LOR_USARTx_DMA_RX_IRQn);
#endif

        HAL_NVIC_SetPriority(LOG_USARTx_IRQn, 0, 2);
        HAL_NVIC_EnableIRQ(LOG_USARTx_IRQn);
    }
    else if (huart->Instance == LOR_USARTx)
    {

        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /* Enable GPIO TX/RX clock */
        LOR_USARTx_TX_GPIO_CLK_ENABLE();
        LOR_USARTx_RX_GPIO_CLK_ENABLE();

        /* Enable USARTx clock */
        LOR_USARTx_CLK_ENABLE();

        /* Enable DMA clock */
        LOR_DMAx_CLK_ENABLE();

        /*##-2- Configure peripheral GPIO ##########################################*/
        /* UART TX GPIO pin configuration  */
        GPIO_InitStruct.Pin = LOR_USARTx_TX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init(LOR_USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

        /* UART RX GPIO pin configuration  */
        GPIO_InitStruct.Pin = LOR_USARTx_RX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

        HAL_GPIO_Init(LOR_USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

#ifdef PLATFORM_USE_DMA
        /*##-3- Configure the DMA ##################################################*/
        /* Configure the DMA handler for Transmission process */
        hdma_lor_tx.Instance = LOR_USARTx_TX_DMA_CHANNEL;
        hdma_lor_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_lor_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_lor_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_lor_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_lor_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_lor_tx.Init.Mode = DMA_NORMAL;
        hdma_lor_tx.Init.Priority = DMA_PRIORITY_LOW;

        HAL_DMA_Init(&hdma_lor_tx);

        /* Associate the initialized DMA handle to the UART handle */
        __HAL_LINKDMA(huart, hdmatx, hdma_lor_tx);

        /* Configure the DMA handler for reception process */
        hdma_lor_rx.Instance = LOR_USARTx_RX_DMA_CHANNEL;
        hdma_lor_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_lor_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_lor_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_lor_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_lor_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_lor_rx.Init.Mode = DMA_NORMAL;
        hdma_lor_rx.Init.Priority = DMA_PRIORITY_HIGH;

        HAL_DMA_Init(&hdma_lor_rx);

        /* Associate the initialized DMA handle to the the UART handle */
        __HAL_LINKDMA(huart, hdmarx, hdma_lor_rx);

        /*##-4- Configure the NVIC for DMA #########################################*/
        /* NVIC configuration for DMA transfer complete interrupt (USART1_TX) */
        HAL_NVIC_SetPriority(LOR_USARTx_DMA_TX_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(LOR_USARTx_DMA_TX_IRQn);

        /* NVIC configuration for DMA transfer complete interrupt (USART1_RX) */
        HAL_NVIC_SetPriority(LOR_USARTx_DMA_RX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(LOR_USARTx_DMA_RX_IRQn);
#endif

        /* NVIC for USART, to catch the TX complete */
        //HAL_NVIC_SetPriority(LOR_USARTx_IRQn, 0, 1);
        //HAL_NVIC_EnableIRQ(LOR_USARTx_IRQn);
    }

    else if (huart->Instance == EXT_USARTx)
    {

        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /* Enable GPIO TX/RX clock */
        EXT_USARTx_TX_GPIO_CLK_ENABLE();
        EXT_USARTx_RX_GPIO_CLK_ENABLE();

        /* Enable USARTx clock */
        EXT_USARTx_CLK_ENABLE();

        /* Enable DMA clock */
        EXT_DMAx_CLK_ENABLE();

        /*##-2- Configure peripheral GPIO ##########################################*/
        /* UART TX GPIO pin configuration  */
        GPIO_InitStruct.Pin = EXT_USARTx_TX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init(EXT_USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

        /* UART RX GPIO pin configuration  */
        GPIO_InitStruct.Pin = EXT_USARTx_RX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

        HAL_GPIO_Init(EXT_USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

#ifdef PLATFORM_USE_DMA
        /*##-3- Configure the DMA ##################################################*/
        /* Configure the DMA handler for Transmission process */
        hdma_ext_tx.Instance = EXT_USARTx_TX_DMA_CHANNEL;
        hdma_ext_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_ext_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_ext_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_ext_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_ext_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_ext_tx.Init.Mode = DMA_NORMAL;
        hdma_ext_tx.Init.Priority = DMA_PRIORITY_LOW;

        HAL_DMA_Init(&hdma_ext_tx);

        /* Associate the initialized DMA handle to the UART handle */
        __HAL_LINKDMA(huart, hdmatx, hdma_ext_tx);

        /* Configure the DMA handler for reception process */
        hdma_ext_rx.Instance = EXT_USARTx_RX_DMA_CHANNEL;
        hdma_ext_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_ext_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_ext_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_ext_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_ext_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_ext_rx.Init.Mode = DMA_NORMAL;
        hdma_ext_rx.Init.Priority = DMA_PRIORITY_HIGH;

        HAL_DMA_Init(&hdma_ext_rx);

        /* Associate the initialized DMA handle to the the UART handle */
        __HAL_LINKDMA(huart, hdmarx, hdma_ext_rx);

        /*##-4- Configure the NVIC for DMA #########################################*/
        /* NVIC configuration for DMA transfer complete interrupt (USART1_TX) */
        HAL_NVIC_SetPriority(EXT_USARTx_DMA_TX_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(EXT_USARTx_DMA_TX_IRQn);

        /* NVIC configuration for DMA transfer complete interrupt (USART1_RX) */
        HAL_NVIC_SetPriority(EXT_USARTx_DMA_RX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(EXT_USARTx_DMA_RX_IRQn);

#endif

        /* NVIC for USART, to catch the TX complete */
        HAL_NVIC_SetPriority(EXT_USARTx_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(EXT_USARTx_IRQn);
    }
    else if (huart->Instance == GPS_USARTx)
    {

        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /* Enable GPIO TX/RX clock */
        GPS_USARTx_TX_GPIO_CLK_ENABLE();
        GPS_USARTx_RX_GPIO_CLK_ENABLE();

        /* Enable USARTx clock */
        GPS_USARTx_CLK_ENABLE();

        /* Enable DMA clock */
        GPS_DMAx_CLK_ENABLE();

        /*##-2- Configure peripheral GPIO ##########################################*/
        /* UART TX GPIO pin configuration  */
        GPIO_InitStruct.Pin = GPS_USARTx_TX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init(GPS_USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

        /* UART RX GPIO pin configuration  */
        GPIO_InitStruct.Pin = GPS_USARTx_RX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

        HAL_GPIO_Init(GPS_USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

#if 0
		/*##-3- Configure the DMA ##################################################*/
		/* Configure the DMA handler for Transmission process */
		hdma_gps_tx.Instance                 = GPS_USARTx_TX_DMA_CHANNEL;
		hdma_gps_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
		hdma_gps_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
		hdma_gps_tx.Init.MemInc              = DMA_MINC_ENABLE;
		hdma_gps_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_gps_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
		hdma_gps_tx.Init.Mode                = DMA_NORMAL;
		hdma_gps_tx.Init.Priority            = DMA_PRIORITY_LOW;

		HAL_DMA_Init(&hdma_gps_tx);

		/* Associate the initialized DMA handle to the UART handle */
		__HAL_LINKDMA(huart, hdmatx, hdma_gps_tx);

		/* Configure the DMA handler for reception process */
		hdma_gps_rx.Instance                 = GPS_USARTx_RX_DMA_CHANNEL;
		hdma_gps_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
		hdma_gps_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
		hdma_gps_rx.Init.MemInc              = DMA_MINC_ENABLE;
		hdma_gps_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_gps_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
		hdma_gps_rx.Init.Mode                = DMA_NORMAL;
		hdma_gps_rx.Init.Priority            = DMA_PRIORITY_HIGH;

		HAL_DMA_Init(&hdma_gps_rx);

		/* Associate the initialized DMA handle to the the UART handle */
		__HAL_LINKDMA(huart, hdmarx, hdma_gps_rx);

		/*##-4- Configure the NVIC for DMA #########################################*/
		/* NVIC configuration for DMA transfer complete interrupt (USART1_TX) */
		HAL_NVIC_SetPriority(GPS_USARTx_DMA_TX_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(GPS_USARTx_DMA_TX_IRQn);

		/* NVIC configuration for DMA transfer complete interrupt (USART1_RX) */
		HAL_NVIC_SetPriority(GPS_USARTx_DMA_RX_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(GPS_USARTx_DMA_RX_IRQn);
#endif
        /* NVIC for USART, to catch the TX complete */
        HAL_NVIC_SetPriority(GPS_USARTx_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(GPS_USARTx_IRQn);
    }
}

/**
  * @brief UART MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
if(huart->Instance == LOG_USARTx)
	{
		/*##-1- Reset peripherals ##################################################*/
		LOG_USARTx_FORCE_RESET();
		LOG_USARTx_RELEASE_RESET();

		/*##-2- Disable peripherals and GPIO Clocks #################################*/
		/* Configure UART Tx as alternate function  */
		HAL_GPIO_DeInit(LOG_USARTx_TX_GPIO_PORT, LOG_USARTx_TX_PIN);
		/* Configure UART Rx as alternate function  */
		HAL_GPIO_DeInit(LOG_USARTx_RX_GPIO_PORT, LOG_USARTx_RX_PIN);
	}
	else if(huart->Instance == LOR_USARTx)
	{

		/*##-1- Reset peripherals ##################################################*/
		LOR_USARTx_FORCE_RESET();
		LOR_USARTx_RELEASE_RESET();

		/*##-2- Disable peripherals and GPIO Clocks #################################*/
		/* Configure USARTx Tx as alternate function  */
		HAL_GPIO_DeInit(LOR_USARTx_TX_GPIO_PORT, LOR_USARTx_TX_PIN);
		/* Configure USARTx Rx as alternate function  */
		HAL_GPIO_DeInit(LOR_USARTx_RX_GPIO_PORT, LOR_USARTx_RX_PIN);

		/*##-3- Disable the DMA #####################################################*/
		/* De-Initialize the DMA channel associated to reception process */
		if(huart->hdmarx != 0)
		{
			HAL_DMA_DeInit(huart->hdmarx);
		}
		/* De-Initialize the DMA channel associated to transmission process */
		if(huart->hdmatx != 0)
		{
			HAL_DMA_DeInit(huart->hdmatx);
		}  

		/*##-4- Disable the NVIC for DMA ###########################################*/
		HAL_NVIC_DisableIRQ(LOR_USARTx_DMA_TX_IRQn);
		HAL_NVIC_DisableIRQ(LOR_USARTx_DMA_RX_IRQn);
	}
	else if(huart->Instance == EXT_USARTx)
	{

		/*##-1- Reset peripherals ##################################################*/
		EXT_USARTx_FORCE_RESET();
		EXT_USARTx_RELEASE_RESET();

		/*##-2- Disable peripherals and GPIO Clocks #################################*/
		/* Configure USARTx Tx as alternate function  */
		HAL_GPIO_DeInit(EXT_USARTx_TX_GPIO_PORT, EXT_USARTx_TX_PIN);
		/* Configure USARTx Rx as alternate function  */
		HAL_GPIO_DeInit(EXT_USARTx_RX_GPIO_PORT, EXT_USARTx_RX_PIN);

		/*##-3- Disable the DMA #####################################################*/
		/* De-Initialize the DMA channel associated to reception process */
		if(huart->hdmarx != 0)
		{
			HAL_DMA_DeInit(huart->hdmarx);
		}
		/* De-Initialize the DMA channel associated to transmission process */
		if(huart->hdmatx != 0)
		{
			HAL_DMA_DeInit(huart->hdmatx);
		}  

		/*##-4- Disable the NVIC for DMA ###########################################*/
		HAL_NVIC_DisableIRQ(EXT_USARTx_DMA_TX_IRQn);
		HAL_NVIC_DisableIRQ(EXT_USARTx_DMA_RX_IRQn);
	}
	else if(huart->Instance == GPS_USARTx)
	{

		/*##-1- Reset peripherals ##################################################*/
		GPS_USARTx_FORCE_RESET();
		GPS_USARTx_RELEASE_RESET();

		/*##-2- Disable peripherals and GPIO Clocks #################################*/
		/* Configure USARTx Tx as alternate function  */
		HAL_GPIO_DeInit(GPS_USARTx_TX_GPIO_PORT, GPS_USARTx_TX_PIN);
		/* Configure USARTx Rx as alternate function  */
		HAL_GPIO_DeInit(GPS_USARTx_RX_GPIO_PORT, GPS_USARTx_RX_PIN);

		/*##-3- Disable the DMA #####################################################*/
		/* De-Initialize the DMA channel associated to reception process */
		if(huart->hdmarx != 0)
		{
			HAL_DMA_DeInit(huart->hdmarx);
		}
		/* De-Initialize the DMA channel associated to transmission process */
		if(huart->hdmatx != 0)
		{
			HAL_DMA_DeInit(huart->hdmatx);
		}  

		/*##-4- Disable the NVIC for DMA ###########################################*/
#if 0     
		HAL_NVIC_DisableIRQ(GPS_USARTx_DMA_TX_IRQn);
		HAL_NVIC_DisableIRQ(GPS_USARTx_DMA_RX_IRQn);
#endif        
	}
}


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
