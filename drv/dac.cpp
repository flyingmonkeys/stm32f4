//
// This file is part of the µOS++ III distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

// For memset, memcpy
#include <string.h>

#include "dac.h"

// For DMA streams
#include "stm32f4xx_hal_dma.h"

// Forward declarations.

static void TIM_TimeBaseStructInit(TIM_Base_InitTypeDef* );


Peripheral_DAC1::Peripheral_DAC1(uint16_t *pOutbuf0 = NULL, uint16_t *pOutbuf1 = NULL, uint16_t bufSize = 0) : Peripheral_GPIO(GPIO_A)
{
    // DAC structures
    DAC_ChannelConfTypeDef DAC_ChannelConf;
    DMA_InitTypeDef DMA_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;

    // Timer structures
    TIM_Base_InitTypeDef TIM_TimeBaseStructure;
    TIM_HandleTypeDef TIM_DAC_Handle;
    TIM_MasterConfigTypeDef TIM_MasterConf;

    // DMA structures
    DMA_HandleTypeDef DMA_DAC_Handle;

    if ( (pOutbuf0 == NULL) || (pOutbuf1 == NULL) || (bufSize == 0) )
    {
        object_valid = false;
    }
    else
    {
        this->pOutbuf0 = pOutbuf0;
        this->pOutbuf1 = pOutbuf1;
        this->bufSize = bufSize;
    }

    // Enable DAC clock
    __DAC_CLK_ENABLE();

    // Configure DMA1 Channel 7, stream 5 for DAC1 use later
    __DMA1_CLK_ENABLE();

    DMA_InitStruct.Channel = DMA_CHANNEL_7;
    DMA_InitStruct.Direction = DMA_MEMORY_TO_PERIPH;
    DMA_InitStruct.Priority = DMA_PRIORITY_VERY_HIGH;
    DMA_InitStruct.PeriphInc = DMA_PINC_DISABLE;
    DMA_InitStruct.MemInc = DMA_MINC_ENABLE;
    DMA_InitStruct.Mode = DMA_CIRCULAR;
    DMA_InitStruct.FIFOMode = DMA_FIFOMODE_DISABLE;
    DMA_InitStruct.MemDataAlignment = DMA_MDATAALIGN_HALFWORD; // uint16
    DMA_InitStruct.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; // 12-bit output
    DMA_InitStruct.MemBurst = DMA_MBURST_SINGLE;
    DMA_InitStruct.PeriphBurst = DMA_PBURST_SINGLE;

    DMA_DAC_Handle.State = HAL_DMA_STATE_RESET;
    DMA_DAC_Handle.Instance = (DMA_Stream_TypeDef *)(DMA1_Stream5_BASE);
    DMA_DAC_Handle.Init = DMA_InitStruct; //c++ does a shallow copy if assigning this way

    HAL_DMA_Init(&DMA_DAC_Handle); // TODO: check return value
    HAL_DMAEx_MultiBufferStart(&DMA_DAC_Handle, (uint32_t)pOutbuf0, (uint32_t)&DAC->DHR12R1, (uint32_t)pOutbuf1, bufSize); // TODO: check return value



    // Configure and Start Timer 6 to be the DAC trigger

    // Enable TIM6 for DAC output timing
    __TIM6_CLK_ENABLE();

    // Configure the timer
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.Prescaler = SystemCoreClock/100000 - 1; // make it 100khz per tick
    TIM_TimeBaseStructure.Period = 100 - 1; // 0..99 (ARR register value), makes it 100khz/100 = 1khz
    TIM_TimeBaseStructure.CounterMode = TIM_COUNTERMODE_UP;

    // Init the timer
    memset(&TIM_DAC_Handle, 0, sizeof(TIM_HandleTypeDef));
    TIM_DAC_Handle.Channel = HAL_TIM_ACTIVE_CHANNEL_1;
    TIM_DAC_Handle.Instance = (TIM_TypeDef *)(TIM6_BASE);
    //TIM_DAC_Handle.Init = TIM_TimeBaseStructure;
    memcpy(&TIM_DAC_Handle.Init, &TIM_TimeBaseStructure, sizeof(TIM_Base_InitTypeDef));
    HAL_TIM_Base_Init(&TIM_DAC_Handle); // TODO: check for HAL_OK return status

    TIM_MasterConf.MasterOutputTrigger = TIM_TRGO_UPDATE;
    TIM_MasterConf.MasterSlaveMode = TIM_SLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&TIM_DAC_Handle, &TIM_MasterConf); // TODO: check return value

    // Start the timer
    HAL_TIM_Base_Start(&TIM_DAC_Handle); // TODO: check return value


    // Init GPIOA for DAC1 (always on PA4)
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init( (GPIO_TypeDef *)(GPIOA_BASE), &GPIO_InitStruct);

 #if 0
    // Init GPIOA for DAC2 (PA5)
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init( (GPIO_TypeDef *)(GPIOA_BASE), &GPIO_InitStruct);
 #endif

    // Init the DAC
    handle.State = HAL_DAC_STATE_RESET;
    handle.Instance = (DAC_TypeDef *)(DAC_BASE);
    HAL_DAC_Init(&handle); // TODO: check return value

    // Trigger the DAC on timer 6
    DAC_ChannelConf.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
 #if 0
    DAC_ChannelConf.DAC_Trigger = DAC_TRIGGER_NONE;
    HAL_DAC_ConfigChannel(&handle, &DAC_ChannelConf, DAC_CHANNEL_2); // TODO: check return value
 #endif
    DAC_ChannelConf.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
    HAL_DAC_ConfigChannel(&handle, &DAC_ChannelConf, DAC_CHANNEL_1); // TODO: check return value

    // Start the DAC
    handle.DMA_Handle1 = &DMA_DAC_Handle;
    if (HAL_OK == HAL_DAC_Start_DMA(&handle, DAC_CHANNEL_1, (uint32_t *)pOutbuf0, bufSize, DAC_ALIGN_12B_R ))
        object_valid = true;
 #if 0
    HAL_DAC_Start(&handle, DAC_CHANNEL_2); // TODO: check return value
 #endif
}

Peripheral_DAC1::~Peripheral_DAC1()
{
    if (object_valid)
        __DAC_CLK_DISABLE();
    object_valid = false;
}

Peripheral_DAC1::Peripheral_DAC1(Peripheral_DAC1 &dac) : Peripheral_GPIO(GPIO_A)
{
    // No Copy Constructors allowed!!
    dac.object_valid = false;
}

// ----------------------------------------------------------------------------

/**
  * @brief  Fills each TIM_TimeBaseInitStruct member with its default value.
  * @param  TIM_TimeBaseInitStruct : pointer to a TIM_TimeBaseInitTypeDef
  *         structure which will be initialized.
  * @retval None
  */
void TIM_TimeBaseStructInit(TIM_Base_InitTypeDef* TIM_TimeBaseInitStruct)
{
  memset(TIM_TimeBaseInitStruct, 0, sizeof(TIM_Base_InitTypeDef));

  /* Set the default configuration */
  TIM_TimeBaseInitStruct->Period = 0xFFFFFFFF;
  TIM_TimeBaseInitStruct->Prescaler = TIM_ETRPRESCALER_DIV1;
  TIM_TimeBaseInitStruct->ClockDivision = TIM_CLOCKDIVISION_DIV1;
  TIM_TimeBaseInitStruct->CounterMode = TIM_COUNTERMODE_UP;
  TIM_TimeBaseInitStruct->RepetitionCounter = 0x0000;
}


