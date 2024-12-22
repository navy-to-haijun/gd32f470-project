#include "bsp_usart.h"

/**
 * @brief       初始化UART0
 */
void usart0_gpio_config(void)
{
    /*使能时钟*/
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);
    /*配置引脚复用功能*/
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_10);
    /*配置为push-pull模式 */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /*配置UART0 */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
}


/**
 * @brief       重定向printf
 * @note        IAR和GCC重定向的函数不一致
 */
#ifdef __ICCARM__  // IAR 环境
int __write(int file, char *ptr, int len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        usart_data_transmit(USART0, (uint8_t)*ptr++);
        while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
    }
    return len;
}
#elif defined(__GNUC__)  // GCC 环境
int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART0, (uint8_t)ch);
    while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
    return ch;
}
#endif



