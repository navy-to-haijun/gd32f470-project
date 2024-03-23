
/**
 * @brief LED.c文件
 * @details 初始化LED相关引脚
 * @author haijun
 * @date 2024-1-31
 * @version v0.1
 * 
 */

#include"led.h"

/**
 * @brief 初始化LED相关引脚
 * 
 */
void led_gpio_config(void)
{
    /*使能时钟*/
    rcu_periph_clock_enable(RCU_GPIOD);
    /*输出模式 浮空模式*/
    gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_7);
    /*推挽输出 50MHz*/
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
}