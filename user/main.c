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
#include "lwrb.h"

int8_t a = 0;

/*创造一个缓冲区*/
uint8_t lwrb_data[8 + 1];
lwrb_t buff;

int main(void)
{
    lwrb_sz_t len;
    uint8_t rw_buff[8];

    systick_config();
    led_gpio_config();
    /*初始化*/
    lwrb_init(&buff, lwrb_data, sizeof(lwrb_data));
    SEGGER_RTT_printf(0, "Read/Write test\n");
    lwrb_reset(&buff);
    /*读操作*/
    len = lwrb_write(&buff, "abc", 3);
    SEGGER_RTT_WriteString(0, buff.buff);
    SEGGER_RTT_printf(0, "\nwrite %d bytes\n", len);
    len = lwrb_write(&buff, "abcdef", 6);
    SEGGER_RTT_WriteString(0, buff.buff);
    SEGGER_RTT_printf(0, "\nwrite %d bytes\n", len);
    len = lwrb_read(&buff, rw_buff, 3);
    SEGGER_RTT_WriteString(0, buff.buff);
    SEGGER_RTT_printf(0, "\nread %d bytes\n", len);
    len = lwrb_write(&buff, "abcdef", 6);
    SEGGER_RTT_WriteString(0, buff.buff);
    SEGGER_RTT_printf(0, "\nwrite %d bytes\n", len);


   while(1)
   {
        gpio_bit_write(GPIOD, GPIO_PIN_7, SET); 
        delay_1ms(500);
        gpio_bit_write(GPIOD, GPIO_PIN_7, RESET); 
        delay_1ms(500);
        a++;
        // SEGGER_RTT_printf(0, "a = %d\n");
   }
}
