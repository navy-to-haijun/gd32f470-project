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
// #include "lcd.h"
#include "mempool.h"

uint8_t *ptr[64];
uint8_t mempool_store[1024];
struct mempool mp_obj;

int main(void)
{
    int8_t  err;
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
    systick_config();
    led_gpio_config();

    usb_intr_config();
    /* 初始化一个内存池*/
    err = mp_init(&mp_obj, &mempool_store, sizeof(mempool_store), 64);
    if(err != 0)
    {
        SEGGER_RTT_printf(0, "内存池初始化失败!\n");
        return -1;
    }
    SEGGER_RTT_printf(0, "内存池初始化成功!\n");
    SEGGER_RTT_printf(0, "内存块的数量 %d\n", mp_obj.block_total_count);

    delay_1ms(5000);
    // /*分配内存块 */
    for(int i = 0; i < 20; i++)
    {
        ptr[i] = mp_alloc(&mp_obj);
        if(ptr[i] == NULL)
        {
            SEGGER_RTT_printf(0, "内存块分配失败!\n");
        }
        else{
            SEGGER_RTT_printf(0, "内存块分配成功， 地址：%d  %x\n", i, ptr[i]);
            /* 写数据 */
            *ptr[i] = i;
        }
    }
    delay_1ms(1000);
    /* 释放内存块 */
    for(int i = 0; i < 15; i++)
    {
        /*读数据*/
        SEGGER_RTT_printf(0, "地址 %x, 值 %d\n", ptr[i], *ptr[i]);
        mp_free(ptr[i]);
    }
    
    /* 再次分配内存块 */
    delay_1ms(1000);
    uint8_t *temp = mp_alloc(&mp_obj);
    *temp = 125;
    SEGGER_RTT_printf(0, "地址 %x, 值 %d\n", temp, *temp);
    
    
    
    
   while(1)
   {

		gpio_bit_toggle(GPIOD, GPIO_PIN_7);
		delay_1ms(50);
		gpio_bit_toggle(GPIOE, GPIO_PIN_3);
        delay_1ms(50);
        // a++;    
		// SEGGER_RTT_printf(0, "a = %d\n", a);
   }
}
