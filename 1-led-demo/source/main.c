/*!
    \file    main.c
    \brief   led spark with systick

    \version 2024-01-15, V3.2.0, firmware for GD32F4xx
*/


#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"

int8_t a = 0;

int main(void)
{
  
    systick_config();
    led_gpio_config();
    
   while(1)
   {
        gpio_bit_toggle(PORT_LED1, PIN_LED1);
        gpio_bit_toggle(PORT_LED2, PIN_LED2);
        gpio_bit_toggle(PORT_LED3, PIN_LED3);
        gpio_bit_toggle(PORT_LED4, PIN_LED4);
        delay_1ms(1000);
        a++;
   }

}
