//
// This file is part of the µOS++ III distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

// For memset, memcpy
#include <string.h>

#include "adc.h"

// For DMA streams
#include "stm32f4xx_hal_dma.h"

// Forward declarations.


//static void GPIO_StructInit(GPIO_InitTypeDef* );
static void ADC_StructInit(ADC_InitTypeDef* );

Peripheral_ADC1::Peripheral_ADC1(uint8_t channel = 1) : Peripheral_GPIO(GPIO_A)
{
    // ADC structures
    ADC_InitTypeDef ADC_InitStruct;
    ADC_ChannelConfTypeDef ADC_ChannelConf;
    DMA_InitTypeDef DMA_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;

    // Enable ADC1, GPIOA, and DMA2 for ADC1 DMA
    __ADC1_CLK_ENABLE();
    __DMA2_CLK_ENABLE();

   // Init GPIOA for ADC1_IN2 (PA2)
   GPIO_StructInit(&GPIO_InitStruct);
   GPIO_InitStruct.Pin = GPIO_PIN_2;
   GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
   GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
   HAL_GPIO_Init( (GPIO_TypeDef *)(GPIOA_BASE), &GPIO_InitStruct);

   pInbuf0 = inbuf0;

   // Configure DMA2 Channel 0, stream 0 (for later ADC1 use)
   DMA_InitStruct.Channel = DMA_CHANNEL_0;
   DMA_InitStruct.Direction = DMA_PERIPH_TO_MEMORY;
   DMA_InitStruct.Priority = DMA_PRIORITY_VERY_HIGH;
   DMA_InitStruct.PeriphInc = DMA_PINC_DISABLE;
   DMA_InitStruct.MemInc = DMA_MINC_ENABLE;
   DMA_InitStruct.Mode = DMA_CIRCULAR;
   DMA_InitStruct.FIFOMode = DMA_FIFOMODE_DISABLE;
   DMA_InitStruct.MemDataAlignment = DMA_MDATAALIGN_HALFWORD; // uint16
   DMA_InitStruct.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; // 12-bit input
   DMA_InitStruct.MemBurst = DMA_MBURST_SINGLE;
   DMA_InitStruct.PeriphBurst = DMA_PBURST_SINGLE;

   DMA_ADC_Handle.State = HAL_DMA_STATE_RESET;
   DMA_ADC_Handle.Instance = (DMA_Stream_TypeDef *)(DMA2_Stream0_BASE);
   DMA_ADC_Handle.Init = DMA_InitStruct; //c++ does a shallow copy if assigning this way

   HAL_DMA_Init(&DMA_ADC_Handle); // TODO check return value
   HAL_DMAEx_MultiBufferStart(&DMA_ADC_Handle, (uint32_t)&ADC1->DR, (uint32_t)&inbuf0, (uint32_t)&inbuf1, INBUFSIZE); // TODO check return value

   // Init ADC1
   ADC_StructInit(&ADC_InitStruct);
   ADC_InitStruct.ContinuousConvMode = ENABLE;
   ADC_InitStruct.DMAContinuousRequests = ENABLE;
   handle.Instance = (ADC_TypeDef *)(ADC1_BASE);
   memcpy(&handle.Init, &ADC_InitStruct, sizeof(ADC_InitTypeDef));
   handle.State = HAL_ADC_STATE_RESET;
   handle.DMA_Handle = &DMA_ADC_Handle;
   HAL_ADC_Init(&handle); // TODO: check for HAL_OK return status

   ADC_ChannelConf.Channel = ADC_CHANNEL_2;
   ADC_ChannelConf.SamplingTime = ADC_SAMPLETIME_56CYCLES;
   ADC_ChannelConf.Rank = 1; // not sure what this is
   HAL_ADC_ConfigChannel(&handle, &ADC_ChannelConf);

// no DMA (manual one-shot sampling)  HAL_ADC_Start(&ADC_Handle);
   if (HAL_OK == HAL_ADC_Start_DMA(&handle, (uint32_t *)&inbuf0, INBUFSIZE))
       object_valid = true;
}

Peripheral_ADC1::~Peripheral_ADC1()
{
    if (object_valid)
        __ADC1_CLK_DISABLE();
    object_valid = false;
}

Peripheral_ADC1::Peripheral_ADC1(Peripheral_ADC1 &adc) : Peripheral_GPIO(GPIO_A)
{
    // No Copy Constructors allowed!!
    adc.object_valid = false;
    adc.pInbuf0 = adc.inbuf0;
}

// ----------------------------------------------------------------------------

/**
  * @brief  Fills each ADC_InitStruct member with its default value.
  * @note   This function is used to initialize the global features of the ADC (
  *         Resolution and Data Alignment), however, the rest of the configuration
  *         parameters are specific to the regular channels group (scan mode
  *         activation, continuous mode activation, External trigger source and
  *         edge, number of conversion in the regular channels group sequencer).
  * @param  ADC_InitStruct: pointer to an ADC_InitTypeDef structure which will
  *         be initialized.
  * @retval None
  */
void ADC_StructInit(ADC_InitTypeDef* ADC_InitStruct)
{
  memset(ADC_InitStruct, 0, sizeof(ADC_InitTypeDef));

  /* Initialize the ADC_Mode member */
  ADC_InitStruct->Resolution = ADC_RESOLUTION12b;

  /* initialize the ADC_ScanConvMode member */
  ADC_InitStruct->ScanConvMode = DISABLE;

  /* Initialize the ADC_ContinuousConvMode member */
  ADC_InitStruct->ContinuousConvMode = DISABLE;

  /* Initialize the ADC_ExternalTrigConvEdge member */
  ADC_InitStruct->ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;

  /* Initialize the ADC_ExternalTrigConv member */
  ADC_InitStruct->ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;

  /* Initialize the ADC_DataAlign member */
  ADC_InitStruct->DataAlign = ADC_DATAALIGN_RIGHT;

  /* Initialize the ADC_NbrOfConversion member */
  ADC_InitStruct->NbrOfConversion = 1;
}

