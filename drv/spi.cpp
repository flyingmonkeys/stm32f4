//
// This file is part of the µOS++ III distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

// For memset, memcpy
#include <string.h>
#include "spi.h"

// Forward declarations.
static void SPI_StructInit(SPI_InitTypeDef* );

Peripheral_SPI1::Peripheral_SPI1(uint32_t mode = 0) : Peripheral_GPIO(GPIO_A), txBuf(0), rxBuf(0)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // Enable SPI1 clock and GPIOA clock (needed for SPI1 pins)
    __SPI1_CLK_ENABLE();

    // Init GPIO for SPI1 CLK (pin A5), MISO (pin A6), MOSI (pin A7)
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init( (GPIO_TypeDef *)(GPIOA_BASE), &GPIO_InitStruct);
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init( (GPIO_TypeDef *)(GPIOA_BASE), &GPIO_InitStruct);

#if 1
    // TODO: move this somewhere else, not really part of a generic SPI construct

    __GPIOE_CLK_ENABLE();
    // Init GPIO for SPI1 CS (pin E3, connected to accelerometer CS)
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init( (GPIO_TypeDef *)(GPIOE_BASE), &GPIO_InitStruct);
#endif

    // Init SPI1
    SPI_StructInit(&SPI_InitStruct);
    memset(&handle, 0, sizeof(SPI_HandleTypeDef));
//handle.Init = SPI_InitStruct;
    memcpy(&handle.Init, &SPI_InitStruct, sizeof(SPI_InitTypeDef));
    handle.Instance = (SPI_TypeDef *)(SPI1_BASE);
    handle.State = HAL_SPI_STATE_RESET; // reset during initialization
    handle.pRxBuffPtr = &(this->rxBuf);
    handle.pTxBuffPtr = &(this->txBuf);
    if (HAL_OK == HAL_SPI_Init(&handle))
        object_valid = true;
}

Peripheral_SPI1::~Peripheral_SPI1()
{
    if (object_valid)
        __SPI1_CLK_DISABLE();
    object_valid = false;
}

Peripheral_SPI1::Peripheral_SPI1(Peripheral_SPI1 &spi) : Peripheral_GPIO(GPIO_A)
{
    // No Copy Contructors allowed!!
    spi.object_valid = false;
    spi.rxBuf = 0;
    spi.txBuf = 0;
}

uint8_t Peripheral_SPI1::read(uint8_t address)
{
    /* Fill output buffer with data */
    SPI1->DR = address;
#if 0
    /* Wait for transmission to complete */
    while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
    /* Wait for received data to complete */
    while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
    /* Wait for SPI to be ready */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));
#endif
    /* Return data from buffer */
    return SPI1->DR;
}

// ----------------------------------------------------------------------------

/**
  * @brief  Fills each SPI_InitStruct member with its default value.
  * @param  SPI_InitStruct: pointer to a SPI_InitTypeDef structure which will
  *         be initialized as a SPI Mode 0 type with prescaler at 32
  * @retval None
  */
void SPI_StructInit(SPI_InitTypeDef* SPI_InitStruct)
{
    memset(SPI_InitStruct, 0, sizeof(SPI_InitTypeDef));
    SPI_InitStruct->Mode = SPI_MODE_MASTER;
    SPI_InitStruct->CLKPhase = SPI_PHASE_1EDGE;
    SPI_InitStruct->CLKPolarity = SPI_POLARITY_LOW;
    SPI_InitStruct->DataSize = SPI_DATASIZE_8BIT;
    SPI_InitStruct->Direction = SPI_DIRECTION_2LINES;
    SPI_InitStruct->FirstBit = SPI_FIRSTBIT_MSB;
    SPI_InitStruct->BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    SPI_InitStruct->CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    SPI_InitStruct->CRCPolynomial = 0x1;
    SPI_InitStruct->NSS = SPI_NSS_SOFT;
}


