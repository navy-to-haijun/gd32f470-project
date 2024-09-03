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
void gpio_config(void)
{
    // ch1 PB3
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_3);
    gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_3);
}
void timer_config(void)
{
    timer_oc_parameter_struct timer_ocintpara;  // 输出配置
    timer_parameter_struct timer_initpara;      // 定时器配置

    /*使能时钟 */
    rcu_periph_clock_enable(RCU_TIMER1);
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
    timer_deinit(TIMER1);

    /*定时器配置*/
    timer_initpara.prescaler         = 199;                 // 分频系数
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 15999;               // 计数周期
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER1,&timer_initpara);

    /*配置通道为输出*/
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_HIGH;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER0,TIMER_CH_0,&timer_ocintpara);

    /*输出比较模式 */
    timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_0, 1000);           // 配置输出比较值
    timer_channel_output_mode_config(TIMER0,TIMER_CH_0,TIMER_OC_MODE_TOGGLE);   // 电平翻转
    timer_channel_output_shadow_config(TIMER0,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);

    // timer_primary_output_config(TIMER0,ENABLE);
    // timer_auto_reload_shadow_enable(TIMER0);
    /*启用中断*/
    timer_interrupt_enable(TIMER0, TIMER_INT_CH1);
    timer_enable(TIMER0);
}


int main(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
    systick_config();
    led_gpio_config();

    usb_gpio_config();
    usb_rcu_config();
    usb_timer_init();


    usb_intr_config();
    
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

/*中断服务函数*/
void TIMER1_IRQHandler(void)
{
    if(timer_interrupt_flag_get(TIMER0, TIMER_INT_CH0) != RESET)
    {
        timer_interrupt_flag_clear(TIMER0, TIMER_INT_CH0);
        pulse_count++;
        timer_disable(TIM2);
    }
}

