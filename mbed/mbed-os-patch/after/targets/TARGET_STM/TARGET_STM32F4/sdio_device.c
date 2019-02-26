/**
 *
 * This sourcecode is derived from STMicroelectronics CubeMX generated code.
 * It is modified to work with mbed RTOS.
 *
 ******************************************************************************
 * @file    bsp_driver_sd.c for F4 (based on stm324x9i_eval_sd.c)
 * @brief   This file includes a generic uSD card driver.
 ******************************************************************************
 * This notice applies to any and all portions of this file
 * that are not between comment pairs USER CODE BEGIN and
 * USER CODE END. Other portions of this file, whether
 * inserted by the user or by software development tools
 * are owned by their respective copyright owners.
 *
 * Copyright (c) 2018 STMicroelectronics International N.V.
 * All rights reserved.
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

#include "sdio_device.h"
#include "platform/mbed_error.h"

/* Extern variables ---------------------------------------------------------*/

SD_HandleTypeDef hsd;
DMA_HandleTypeDef hdma_sdio_rx;
DMA_HandleTypeDef hdma_sdio_tx;

// simple flags for DMA pending signaling
volatile uint8_t SD_DMA_ReadPendingState  = SD_TRANSFER_OK;
volatile uint8_t SD_DMA_WritePendingState  = SD_TRANSFER_OK;


/* DMA Handlers are global, there is only one SDIO interface */

/**
* @brief This function handles SDIO global interrupt.
*/
void _SDIO_IRQHandler(void)
{
    HAL_SD_IRQHandler(&hsd);
}

/**
* @brief This function handles DMAx stream_n global interrupt. DMA Rx
*/
void _DMA_Stream_Rx_IRQHandler(void)
{
  HAL_DMA_IRQHandler(hsd.hdmarx);
}

/**
* @brief This function handles DMAx stream_n global interrupt. DMA Tx
*/
void _DMA_Stream_Tx_IRQHandler(void)
{
  HAL_DMA_IRQHandler(hsd.hdmatx);
}

/**
 *
 * @param hsd:  Handle for SD handle Structure definition
 */
void HAL_SD_MspInit(SD_HandleTypeDef *hsd) {
    IRQn_Type IRQn;
    GPIO_InitTypeDef GPIO_InitStruct;

    if (hsd->Instance == SDIO) {
        /* Peripheral clock enable */
        __HAL_RCC_SDIO_CLK_ENABLE();
        __HAL_RCC_DMA2_CLK_ENABLE();

        /* Enable GPIOs clock */
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();

        /**SDIO GPIO Configuration
         PC12     ------> SDIO_CK
         PC11     ------> SDIO_D3
         PC10     ------> SDIO_D2
         PD2     ------> SDIO_CMD
         PC9     ------> SDIO_D1
         PC8     ------> SDIO_D0
         */
        GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* NVIC configuration for SDIO interrupts */
        IRQn = SDIO_IRQn;
        HAL_NVIC_SetPriority(IRQn, 0x0E, 0);
        NVIC_SetVector(IRQn, (uint32_t) &_SDIO_IRQHandler);
        HAL_NVIC_EnableIRQ(IRQn);

        /* SDIO DMA Init */
        /* SDIO_RX Init */
        hdma_sdio_rx.Instance = DMA2_Stream3;
        hdma_sdio_rx.Init.Channel = DMA_CHANNEL_4;
        hdma_sdio_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_sdio_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_sdio_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_sdio_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_sdio_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
        hdma_sdio_rx.Init.Mode = DMA_PFCTRL;
        hdma_sdio_rx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_sdio_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
        hdma_sdio_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        hdma_sdio_rx.Init.MemBurst = DMA_MBURST_INC4;
        hdma_sdio_rx.Init.PeriphBurst = DMA_PBURST_INC4;
        if (HAL_DMA_Init(&hdma_sdio_rx) != HAL_OK) {
            error("SDIO DMA Init error at %d in %s", __FILE__, __LINE__);
        }

        __HAL_LINKDMA(hsd, hdmarx, hdma_sdio_rx);

        /* Deinitialize the stream for new transfer */
        HAL_DMA_DeInit(&hdma_sdio_rx);

        /* Configure the DMA stream */
        HAL_DMA_Init(&hdma_sdio_rx);


        /* SDIO_TX Init */
        hdma_sdio_tx.Instance = DMA2_Stream6;
        hdma_sdio_tx.Init.Channel = DMA_CHANNEL_4;
        hdma_sdio_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_sdio_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_sdio_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_sdio_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_sdio_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
        hdma_sdio_tx.Init.Mode = DMA_PFCTRL;
        hdma_sdio_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_sdio_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
        hdma_sdio_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        hdma_sdio_tx.Init.MemBurst = DMA_MBURST_INC4;
        hdma_sdio_tx.Init.PeriphBurst = DMA_PBURST_INC4;
        if (HAL_DMA_Init(&hdma_sdio_tx) != HAL_OK) {
            error("SDIO DMA Init error at %d in %s", __FILE__, __LINE__);
        }

        __HAL_LINKDMA(hsd, hdmatx, hdma_sdio_tx);

        /* Deinitialize the stream for new transfer */
        HAL_DMA_DeInit(&hdma_sdio_tx);

        /* Configure the DMA stream */
        HAL_DMA_Init(&hdma_sdio_tx);

        /* Enable NVIC for DMA transfer complete interrupts */
        IRQn = DMA2_Stream3_IRQn;
        NVIC_SetVector(IRQn, (uint32_t) &_DMA_Stream_Rx_IRQHandler);
        HAL_NVIC_SetPriority(IRQn, 0x0F, 0);
        HAL_NVIC_EnableIRQ(IRQn);

        IRQn = DMA2_Stream6_IRQn;
        NVIC_SetVector(IRQn, (uint32_t) &_DMA_Stream_Tx_IRQHandler);
        HAL_NVIC_SetPriority(IRQn, 0x0F, 0);
        HAL_NVIC_EnableIRQ(IRQn);
    }
}

/**
 *
 * @param hsd:  Handle for SD handle Structure definition
 */
void HAL_SD_MspDeInit(SD_HandleTypeDef *hsd) {

    if (hsd->Instance == SDIO) {
        /* Peripheral clock disable */
        __HAL_RCC_SDIO_CLK_DISABLE();

        /**SDIO GPIO Configuration
         PC12     ------> SDIO_CK
         PC11     ------> SDIO_D3
         PC10     ------> SDIO_D2
         PD2     ------> SDIO_CMD
         PC9     ------> SDIO_D1
         PC8     ------> SDIO_D0
         */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12 | GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_8);

        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

        /* SDIO DMA DeInit */
        HAL_DMA_DeInit(hsd->hdmarx);
        HAL_DMA_DeInit(hsd->hdmatx);
    }

}

/**
 * @brief  DeInitializes the SD MSP.
 * @param  hsd: SD handle
 * @param  Params : pointer on additional configuration parameters, can be NULL.
 */
__weak void SD_MspDeInit(SD_HandleTypeDef *hsd, void *Params) {
    static DMA_HandleTypeDef dma_rx_handle;
    static DMA_HandleTypeDef dma_tx_handle;

    /* Disable NVIC for DMA transfer complete interrupts */
    HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn);
    HAL_NVIC_DisableIRQ(DMA2_Stream6_IRQn);

    /* Deinitialize the stream for new transfer */
    dma_rx_handle.Instance = DMA2_Stream3;
    HAL_DMA_DeInit(&dma_rx_handle);

    /* Deinitialize the stream for new transfer */
    dma_tx_handle.Instance = DMA2_Stream6;
    HAL_DMA_DeInit(&dma_tx_handle);

    /* Disable NVIC for SDIO interrupts */
    HAL_NVIC_DisableIRQ(SDIO_IRQn);

    /* Disable SDIO clock */
    __HAL_RCC_SDIO_CLK_DISABLE();
}

/**
 * @brief  Initializes the SD card device.
 * @retval SD status
 */
uint8_t SD_Init(void) {
    uint8_t sd_state = MSD_OK;

    hsd.Instance = SDIO;
    hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
    hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
    hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd.Init.ClockDiv = 0;
    // !!! clock must be slower in polling mode if compiled w/o optimization !!!

    /* HAL SD initialization */
    sd_state = HAL_SD_Init(&hsd);
    /* Configure SD Bus width (4 bits mode selected) */
    if (sd_state == MSD_OK) {
        /* Enable wide operation */
        if (HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B) != HAL_OK) {
            sd_state = MSD_ERROR;
        }
    }

    return sd_state;
}

/**
 * @brief  DeInitializes the SD card device.
 * @retval SD status
 */
uint8_t SD_DeInit(void) {
    uint8_t sd_state = MSD_OK;

    hsd.Instance = SDIO;

    /* HAL SD deinitialization */
    if (HAL_SD_DeInit(&hsd) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    /* Msp SD deinitialization */
    hsd.Instance = SDIO;
    SD_MspDeInit(&hsd, NULL);

    return sd_state;
}

/**
 * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
 * @param  pData: Pointer to the buffer that will contain the data to transmit
 * @param  ReadAddr: Address from where data is to be read
 * @param  NumOfBlocks: Number of SD blocks to read
 * @param  Timeout: Timeout for read operation
 * @retval SD status
 */
uint8_t SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout) {
    uint8_t sd_state = MSD_OK;

    if (HAL_SD_ReadBlocks(&hsd, (uint8_t *) pData, ReadAddr, NumOfBlocks, Timeout) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

/**
 * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
 * @param  pData: Pointer to the buffer that will contain the data to transmit
 * @param  WriteAddr: Address from where data is to be written
 * @param  NumOfBlocks: Number of SD blocks to write
 * @param  Timeout: Timeout for write operation
 * @retval SD status
 */
uint8_t SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout) {
    uint8_t sd_state = MSD_OK;

    if (HAL_SD_WriteBlocks(&hsd, (uint8_t *) pData, WriteAddr, NumOfBlocks, Timeout) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

/**
 * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
 * @param  pData: Pointer to the buffer that will contain the data to transmit
 * @param  ReadAddr: Address from where data is to be read
 * @param  NumOfBlocks: Number of SD blocks to read
 * @retval SD status
 */
uint8_t SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks) {
    uint8_t sd_state = MSD_OK;
    SD_DMA_ReadPendingState = SD_TRANSFER_BUSY;

    /* Read block(s) in DMA transfer mode */
    if (HAL_SD_ReadBlocks_DMA(&hsd, (uint8_t *) pData, ReadAddr, NumOfBlocks) != HAL_OK) {
        sd_state = MSD_ERROR;
        SD_DMA_ReadPendingState  = SD_TRANSFER_OK;
    }

    return sd_state;
}

/**
 * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
 * @param  pData: Pointer to the buffer that will contain the data to transmit
 * @param  WriteAddr: Address from where data is to be written
 * @param  NumOfBlocks: Number of SD blocks to write
 * @retval SD status
 */
uint8_t SD_WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks) {
    uint8_t sd_state = MSD_OK;
    SD_DMA_WritePendingState = SD_TRANSFER_BUSY;

    /* Write block(s) in DMA transfer mode */
    if (HAL_SD_WriteBlocks_DMA(&hsd, (uint8_t *) pData, WriteAddr, NumOfBlocks) != HAL_OK) {
        sd_state = MSD_ERROR;
        SD_DMA_WritePendingState  = SD_TRANSFER_OK;
    }

    return sd_state;
}

/**
 * @brief  Erases the specified memory area of the given SD card.
 * @param  StartAddr: Start byte address
 * @param  EndAddr: End byte address
 * @retval SD status
 */
uint8_t SD_Erase(uint32_t StartAddr, uint32_t EndAddr) {
    uint8_t sd_state = MSD_OK;

    if (HAL_SD_Erase(&hsd, StartAddr, EndAddr) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

/**
 * @brief  Gets the current SD card data status.
 * @param  None
 * @retval Data transfer state.
 *          This value can be one of the following values:
 *            @arg  SD_TRANSFER_OK: No data transfer is acting
 *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
 */
uint8_t SD_GetCardState(void) {
    return ((HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

/**
 * @brief  Get SD information about specific SD card.
 * @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
 * @retval None
 */
void SD_GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo) {
    /* Get SD card Information */
    HAL_SD_GetCardInfo(&hsd, CardInfo);
}

/**
 * @brief  Check if a DMA operation is pending
 * @retval DMA operation is pending
 *          This value can be one of the following values:
 *            @arg  SD_TRANSFER_OK: No data transfer is acting
 *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
 */
uint8_t SD_DMA_ReadPending(void) {
    return SD_DMA_ReadPendingState;
}

/**
 * @brief  Check if a DMA operation is pending
 * @retval DMA operation is pending
 *          This value can be one of the following values:
 *            @arg  SD_TRANSFER_OK: No data transfer is acting
 *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
 */
uint8_t SD_DMA_WritePending(void) {
    return SD_DMA_WritePendingState;
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd Pointer SD handle
  * @retval None
  */
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
    SD_DMA_ReadPendingState  = SD_TRANSFER_OK;
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hsd Pointer to SD handle
  * @retval None
  */
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
    SD_DMA_WritePendingState  = SD_TRANSFER_OK;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/