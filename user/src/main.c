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

int8_t a = 0;
usb_core_driver cdc_acm;


int main(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
    systick_config();
    led_gpio_config();

    usb_gpio_config();
    usb_rcu_config();
    usb_timer_init();

    usbd_init(&cdc_acm, USB_CORE_ENUM_FS, &cdc_desc, &cdc_class);

    usb_intr_config();
    
   while(1)
   {

		gpio_bit_toggle(GPIOD, GPIO_PIN_7);
		delay_1ms(50);
		gpio_bit_toggle(GPIOE, GPIO_PIN_3);
        delay_1ms(50);
        if (USBD_CONFIGURED == cdc_acm.dev.cur_status)
        {
            if (0U == cdc_acm_check_ready(&cdc_acm))
            {
                cdc_acm_data_receive(&cdc_acm);
                usb_cdc_handler *cdc = ( usb_cdc_handler *)cdc_acm.dev.class_data[CDC_COM_INTERFACE];
                // cdc->data[cdc->receive_length] = 0;
                SEGGER_RTT_printf(0, "len = %d, str = %s\n", cdc->packet_receive, cdc->data);
                SEGGER_RTT_printf(0, "len = %d, str = %s\n", cdc->receive_length, cdc->data);
            }
            else
            {
                cdc_acm_data_send(&cdc_acm);
            }
        }
        // a++;    
		// SEGGER_RTT_printf(0, "a = %d\n", a);
   }
}
