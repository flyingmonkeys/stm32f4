//
// This file is part of the µOS++ III distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

// For memset, memcpy
#include <string.h>

#include "gpio.h"

/* Static class members must be declared outside the class, and globally!! */
int Peripheral_GPIO::numGPIOports[NUM_GPIO_TYPES] = {0, 0, 0, 0, 0, 0}; // IMPORTANT! Static class members need to be declared/allocated globally, outside the class definition!!!

// Forward declarations.

Peripheral_GPIO::Peripheral_GPIO(e_GpioType gpio_in = GPIO_A) : object_valid(false)
{
//    GPIO_InitTypeDef GPIO_InitStruct;

    gpio_x = gpio_in;

    // Enable GPIOx
    switch(gpio_x)
    {
        case GPIO_A:
            __GPIOA_CLK_ENABLE();
            break;
        case GPIO_B:
            __GPIOB_CLK_ENABLE();
            break;
        case GPIO_C:
            __GPIOC_CLK_ENABLE();
            break;
        case GPIO_D:
            __GPIOD_CLK_ENABLE();
            break;
        case GPIO_E:
            __GPIOE_CLK_ENABLE();
            break;
        case GPIO_H:
            __GPIOH_CLK_ENABLE();
            break;
        default:
            break;
    }

   ++numGPIOports[(int)gpio_x];
}

Peripheral_GPIO::~Peripheral_GPIO()
{
    if (--numGPIOports[(int)gpio_x] <= 0)
    {
        // Enable GPIOx
        switch(gpio_x)
        {
            case GPIO_A:
                __GPIOA_CLK_DISABLE();
                break;
            case GPIO_B:
                __GPIOB_CLK_DISABLE();
                break;
            case GPIO_C:
                __GPIOC_CLK_DISABLE();
                break;
            case GPIO_D:
                __GPIOD_CLK_DISABLE();
                break;
            case GPIO_E:
                __GPIOE_CLK_DISABLE();
                break;
            case GPIO_H:
                __GPIOH_CLK_DISABLE();
                break;
            default:
                break;
        }
    }
}

Peripheral_GPIO::Peripheral_GPIO(Peripheral_GPIO &gpio)
{
    // No Copy Constructors allowed!!
    gpio.object_valid = false;

    // To avoid compiler warnings
    gpio_x = GPIO_A;
}

bool Peripheral_GPIO::is_object_valid(void) const
{
    return(object_valid);
}

// ----------------------------------------------------------------------------


/**
   * @brief  Fills each GPIO_InitStruct member with its default value.
   * @param  GPIO_InitStruct : pointer to a GPIO_InitTypeDef structure which will be initialized.
   * @retval None
   */
void Peripheral_GPIO::GPIO_StructInit(GPIO_InitTypeDef* GPIO_InitStruct)
{
    memset(GPIO_InitStruct, 0, sizeof(GPIO_InitTypeDef));

    /* Reset GPIO init structure parameters values */
    GPIO_InitStruct->Pin  = GPIO_PIN_All;
    GPIO_InitStruct->Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct->Speed = GPIO_SPEED_MEDIUM;
    GPIO_InitStruct->Alternate = GPIO_AF0_RTC_50Hz;
    GPIO_InitStruct->Pull = GPIO_NOPULL;
}

