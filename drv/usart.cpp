//
// This file is part of the µOS++ III distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

// For memset, memcpy
#include <string.h>

#include "usart.h"

// Forward declarations.
static void USART_StructInit(USART_InitTypeDef* );

Peripheral_USART3::Peripheral_USART3(uint32_t baudrate = 9600) : Peripheral_GPIO(GPIO_B)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;

    // Enable USART3 clock and GPIOB clock (needed for USART3 pins)
    __USART3_CLK_ENABLE();

    // Init GPIO for USART3 Tx (pin B10)
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init( (GPIO_TypeDef *)(GPIOB_BASE), &GPIO_InitStruct);

    // Init GPIO for USART3 Rx (pin B11)
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( (GPIO_TypeDef *)(GPIOB_BASE), &GPIO_InitStruct);

    // Init USART3
    USART_StructInit(&USART_InitStruct);
    USART_InitStruct.BaudRate = baudrate;
    //USART1_Handle.Init = USART_InitStruct;
    memcpy(&handle.Init, &USART_InitStruct, sizeof(USART_InitTypeDef));
    handle.Instance = (USART_TypeDef *)(USART3_BASE);
    handle.State = HAL_USART_STATE_RESET; // reset during initialization
    if (HAL_OK == HAL_USART_Init(&handle))
        object_valid = true;
}

Peripheral_USART3::~Peripheral_USART3()
{
    if (object_valid)
        __USART3_CLK_DISABLE();
    object_valid = false;
}

Peripheral_USART3::Peripheral_USART3(Peripheral_USART3 &usart) : Peripheral_GPIO(GPIO_B)
{
    // No Copy Contructors allowed!!
    usart.object_valid = false;
}

// ----------------------------------------------------------------------------


/**
  * @brief  Fills each USART_InitStruct member with its default value.
  * @param  USART_InitStruct: pointer to a USART_InitTypeDef structure which will
  *         be initialized.
  * @retval None
  */
void USART_StructInit(USART_InitTypeDef* USART_InitStruct)
{
    memset(USART_InitStruct, 0, sizeof(USART_InitTypeDef));

    /* USART_InitStruct members default value */
    USART_InitStruct->BaudRate = 9600;
    USART_InitStruct->WordLength = USART_WORDLENGTH_8B;
    USART_InitStruct->StopBits = USART_STOPBITS_1;
    USART_InitStruct->Parity = USART_PARITY_NONE ;
    USART_InitStruct->Mode = USART_MODE_TX_RX;
    USART_InitStruct->CLKPolarity = USART_POLARITY_HIGH;
    USART_InitStruct->CLKPhase = USART_PHASE_1EDGE;
    USART_InitStruct->CLKLastBit = USART_LASTBIT_DISABLE;

//  USART_InitStruct->HardwareFlowControl = USART_HardwareFlowControl_None;
}


