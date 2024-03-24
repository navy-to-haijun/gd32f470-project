/*!
    \file    main.c
    \brief   led spark with systick

    \version 2024-01-15, V3.2.0, firmware for GD32F4xx
*/
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "SEGGER_RTT.h"

int8_t a = 0;

int main(void)
{
  
    systick_config();
    led_gpio_config();
    
   while(1)
   {
        gpio_bit_write(GPIOD, GPIO_PIN_7, SET); 
        delay_1ms(500);
        gpio_bit_write(GPIOD, GPIO_PIN_7, RESET); 
        delay_1ms(500);
        a++;
        SEGGER_RTT_printf(0, "a = %d\n");
   }
}
