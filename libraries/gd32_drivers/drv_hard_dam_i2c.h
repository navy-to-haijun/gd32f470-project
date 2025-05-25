/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes

 */

#ifndef __DRV_HARD_DMA_I2C_H__
#define __DRV_HARD_DMA_I2C_H__

// #include <rtdevice.h>
#include <rtthread.h>
#include <board.h>

// #include <gd32f4xx_libopt.h>


#define I2C_WAIT_LIMIT  5000        // I2C 超时判断阈值

#define WAIT_FLAG_WITH_TIMEOUT(flag_expr, result_var)                   \
    do {                                                                 \
        volatile int16_t __wait_loop = I2C_WAIT_LIMIT;                   \
        while ((flag_expr))                                             \
        {                                                                \
            if (--__wait_loop == 0)                                      \
            {                                                            \
                result_var = -RT_ETIMEOUT;                               \
                break;                                                   \
            }                                                            \
        }                                                                \
    } while (0)

typedef struct
{
    rcu_periph_enum rcu;
    uint32_t alt_func_num;
    uint32_t port;
    uint32_t Pin;
}gd32_i2c_gpio_t;


typedef struct
{
    struct rt_i2c_bus_device i2c_bus;    // RT-Thread 的 I2C 总线设备对象
    const char *bus_name;            // i2c 设备名称

    uint32_t i2c_periph;             // I2C 外设基地址：I2C0、I2C1、I2C2
    rcu_periph_enum i2c_rcu;       // I2C 外设时钟

    uint32_t dma_periph;          // DMA 外设基地址：DMA0、DMA1
    rcu_periph_enum dma_rcu;       // DMA 外设时钟
    gd32_i2c_gpio_t gpio_scl;            // SCL 引脚配置
    gd32_i2c_gpio_t gpio_sda;            // SDA 引脚配置

    uint32_t dma_tx_channel;         // 对应 TX DMA 通道号
    uint32_t dma_rx_channel;         // 对应 RX DMA 通道号

    IRQn_Type dma_tx_irq;            // TX DMA 中断号
    IRQn_Type dma_rx_irq;            // RX DMA 中断号

    dma_subperipheral_enum dma_tx_subperiph;  // DMA Peripheral
    dma_subperipheral_enum dam_rx_subperiph;  // DMA Peripheral

    struct rt_completion tx_comp;    // TX DMA 完成的完成信号量
    struct rt_completion rx_comp;    // RX DMA 完成的完成信号量

}gd32_i2c_bus_t;

#endif /* __DRV_I2C_H__ */
