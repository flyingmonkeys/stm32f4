//
// This file is part of the µOS++ III distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

// For memset, memcpy
#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_cortex.h"

// For TIM interfacing
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_tim.h"

#include "pwm.h"

// For debug output
#include "stm32f4xx_hal_usart.h"
extern USART_HandleTypeDef USART_Handle;

// Forward declarations.


static void TIM_TimeBaseStructInit(TIM_Base_InitTypeDef* );
static void TIM_OCStructInit(TIM_OC_InitTypeDef* );

Peripheral_PWM::Peripheral_PWM(uint32_t period = 1000, uint32_t pulseWidth = 70) : Peripheral_GPIO(GPIO_A)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_Base_InitTypeDef TIM_TimeBaseStructure;
    TIM_OC_InitTypeDef TIM_OCInitStructure;
uint8_t charString[80],numChars;
    uint32_t timerPeriod = 1000;//period;
    uint32_t timerPulseWidth = 70;//pulseWidth;

numChars = sprintf((char *)charString, "period = %u\r\n", period);
HAL_USART_Transmit(&USART_Handle, charString, numChars, HAL_MAX_DELAY - 1);
numChars = sprintf((char *)charString, "pulseWidth = %u\r\n", pulseWidth);
HAL_USART_Transmit(&USART_Handle, charString, numChars, HAL_MAX_DELAY - 1);

    // Enable TIM2 and GPIOA pins (for TIM2 PWM output)
   __TIM2_CLK_ENABLE();

   // Init GPIOA for TIM2 PWM output (pin A1)
   GPIO_StructInit(&GPIO_InitStruct);
   GPIO_InitStruct.Pin = GPIO_PIN_1;
   GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
   GPIO_InitStruct.Pull = GPIO_PULLUP;
   HAL_GPIO_Init( (GPIO_TypeDef *)(GPIOA_BASE), &GPIO_InitStruct);

   TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
   TIM_TimeBaseStructure.Prescaler = SystemCoreClock/100000 - 1; // 0..159
   TIM_TimeBaseStructure.Period = timerPeriod - 1; // 0..999
   TIM_TimeBaseStructure.CounterMode = TIM_COUNTERMODE_UP;

   TIM_OCStructInit(&TIM_OCInitStructure);
   TIM_OCInitStructure.OCMode = TIM_OCMODE_PWM1;
   TIM_OCInitStructure.OCIdleState = TIM_OCIDLESTATE_SET;
   TIM_OCInitStructure.Pulse = timerPulseWidth; // pulse width (relative to period)

   handle.Channel = HAL_TIM_ACTIVE_CHANNEL_2;
   handle.Instance = (TIM_TypeDef *)(TIM2_BASE);
   handle.State = HAL_TIM_STATE_RESET; // reset during initialization
   //TIM_Handle.Init = TIM_TimeBaseStructure;
   memcpy(&handle.Init, &TIM_TimeBaseStructure, sizeof(TIM_Base_InitTypeDef));
   if (HAL_OK == HAL_TIM_PWM_Init(&handle))
   {
       object_valid = true;

       HAL_TIM_PWM_ConfigChannel(&handle, &TIM_OCInitStructure, TIM_CHANNEL_2);

       // Start PWM signal here
       HAL_TIM_PWM_Start(&handle, TIM_CHANNEL_2);
   }
}

Peripheral_PWM::~Peripheral_PWM()
{
    if (object_valid)
        __TIM2_CLK_DISABLE();
    object_valid = false;
}

Peripheral_PWM::Peripheral_PWM(Peripheral_PWM &pwm) : Peripheral_GPIO(GPIO_A)
{
    // No Copy Constructors allowed!!
    pwm.object_valid = false;
}

bool Peripheral_PWM::SetDutyCycle(uint8_t dutyCycle)
{
    bool returnType = false;

    if (dutyCycle < 100)
    {
        /* Set the Capture Compare Register value */
        (handle.Instance)->CCR2 = uint32_t(dutyCycle * (handle.Init.Period + 1)) / 100;

        returnType = true;
    }

    return(returnType);
}

bool Peripheral_PWM::SetPulseWidth(uint32_t pulseWidth)
{
    bool returnType = false;

    if (pulseWidth < handle.Init.Period + 1)
    {
        (handle.Instance)->CCR2 = pulseWidth;

        returnType = true;
    }

    return (returnType);
}

Driver_Servo::Driver_Servo(uint16_t initialAngle = 90)
{
    if ((initialAngle < 30) || (initialAngle > 210))
        initialAngle = 90;

    angle = initialAngle;
}

Driver_Servo::~Driver_Servo()
{

}

uint16_t Driver_Servo::getAngle(void) const
{
    return(angle);
}

void Driver_Servo::setAngle(uint16_t desiredAngle)
{
    if ((desiredAngle < 30) || (desiredAngle > 210))
        return;

    angle = desiredAngle;
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

/**
  * @brief  Fills each TIM_OCInitStruct member with its default value.
  * @param  TIM_OCInitStruct: pointer to a TIM_OCInitTypeDef structure which will
  *         be initialized.
  * @retval None
  */
void TIM_OCStructInit(TIM_OC_InitTypeDef* TIM_OCInitStruct)
{
  memset(TIM_OCInitStruct, 0, sizeof(TIM_OC_InitTypeDef));

  /* Set the default configuration */
  TIM_OCInitStruct->OCMode = TIM_OCMODE_TIMING;
  TIM_OCInitStruct->Pulse = 0x00000000;
  TIM_OCInitStruct->OCFastMode = TIM_OCFAST_DISABLE;
  TIM_OCInitStruct->OCPolarity = TIM_OCPOLARITY_HIGH;
  TIM_OCInitStruct->OCNPolarity = TIM_OCNPOLARITY_HIGH;
  TIM_OCInitStruct->OCIdleState = TIM_OCIDLESTATE_RESET;
  TIM_OCInitStruct->OCNIdleState = TIM_OCNIDLESTATE_RESET;
}
