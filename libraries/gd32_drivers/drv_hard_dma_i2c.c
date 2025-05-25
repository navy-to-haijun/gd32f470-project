/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes

 */

#include "drv_hard_dam_i2c.h"

/*定义日志显示级别*/
#define LOG_TAG "drv.i2c.hw"
#define DBG_LEVEL           DBG_WARNING
#include <rtdbg.h>

#define BSP_USING_I2C0
#define BSP_USING_I2C1
#define BSP_USING_I2C2

/*i2参数配置表*/
static  gd32_i2c_bus_t i2c_buses[] =
{
    #ifdef BSP_USING_I2C0
    {
        .bus_name = "i2c0",
        .i2c_periph = I2C0,
        .gpio_scl = {.Pin = GPIO_PIN_8, .alt_func_num = GPIO_AF_4, .port = GPIOB, .rcu = RCU_GPIOB},
        .gpio_sda = {.Pin = GPIO_PIN_9, .alt_func_num = GPIO_AF_4, .port = GPIOB, .rcu = RCU_GPIOB},
        .i2c_rcu = RCU_I2C0,
        .dma_periph = DMA0,
        .dma_rcu = RCU_DMA0,
        .dma_tx_channel = DMA_CH6,
        .dma_rx_channel = DMA_CH5,
        .dma_tx_irq = DMA0_Channel6_IRQn,
        .dma_rx_irq = DMA0_Channel5_IRQn,
        .dma_tx_subperiph = DMA_SUBPERI1,
        .dam_rx_subperiph = DMA_SUBPERI1,
    },
    #endif
    #ifdef BSP_USING_I2C1
    {
        .bus_name = "i2c1",
        .i2c_periph = I2C1,
        .gpio_scl = {.Pin = GPIO_PIN_10, .alt_func_num = GPIO_AF_4, .port = GPIOB, .rcu = RCU_GPIOB},
        .gpio_sda = {.Pin = GPIO_PIN_3, .alt_func_num = GPIO_AF_9, .port = GPIOB, .rcu = RCU_GPIOB},
        .i2c_rcu = RCU_I2C1,
        .dma_periph = DMA0,
        .dma_rcu = RCU_DMA0,
        .dma_tx_channel = DMA_CH7,
        .dma_rx_channel = DMA_CH3,
        .dma_tx_irq = DMA0_Channel7_IRQn,
        .dma_rx_irq = DMA0_Channel3_IRQn,
        .dma_tx_subperiph = DMA_SUBPERI7,
        .dam_rx_subperiph = DMA_SUBPERI7,
    },
    #endif
    #ifdef BSP_USING_I2C2
    {
        .bus_name = "i2c2",
        .i2c_periph = I2C2,
        .gpio_scl = {.Pin = GPIO_PIN_9, .alt_func_num = GPIO_AF_4, .port = GPIOC, .rcu = RCU_GPIOC},
        .gpio_sda = {.Pin = GPIO_PIN_8, .alt_func_num = GPIO_AF_4, .port = GPIOA, .rcu = RCU_GPIOA},
        .i2c_rcu = RCU_I2C1,
        .dma_periph = DMA0,
        .dma_rcu = RCU_DMA0,
        .dma_tx_channel = DMA_CH4,
        .dma_rx_channel = DMA_CH2,
        .dma_tx_irq = DMA0_Channel4_IRQn,
        .dma_rx_irq = DMA0_Channel2_IRQn,
        .dma_tx_subperiph = DMA_SUBPERI3,
        .dam_rx_subperiph = DMA_SUBPERI3,
    }
    #endif
};
/**
 * * @brief  I2C GPIO 初始化
 */
static void gd32_i2c_gpio_config(gd32_i2c_bus_t *bus)
{
    /*使能时钟*/
    rcu_periph_clock_enable(bus->i2c_rcu);
    rcu_periph_clock_enable(bus->gpio_scl.rcu);
    rcu_periph_clock_enable(bus->gpio_sda.rcu);
    /*设置引脚复用 */
    gpio_af_set(bus->gpio_scl.port, bus->gpio_scl.alt_func_num, bus->gpio_scl.Pin);
    gpio_af_set(bus->gpio_sda.port, bus->gpio_sda.alt_func_num, bus->gpio_sda.Pin);
    /*内部上拉*/
    gpio_mode_set(bus->gpio_scl.port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, bus->gpio_scl.Pin);
    gpio_mode_set(bus->gpio_sda.port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, bus->gpio_sda.Pin);
    /*开漏输出 速度为50M*/
    gpio_output_options_set(bus->gpio_scl.port, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, bus->gpio_scl.Pin);
    gpio_output_options_set(bus->gpio_sda.port, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, bus->gpio_sda.Pin);
}
/*
 * * @brief  I2C 初始化
 */
static void gd32_i2c_config(gd32_i2c_bus_t *bus)
{
    /*设置时钟频率*/
    i2c_clock_config(bus->i2c_periph, 100000, I2C_DTCY_2);
    /*设置IIC的地址：I2C模式 7位地址*/
    i2c_mode_addr_config(bus->i2c_periph, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0);
    /*使能I2C*/
    i2c_enable(bus->i2c_periph);
    /*使能ACK*/
    i2c_ack_config(bus->i2c_periph, I2C_ACK_ENABLE);
    /*禁止DMA（使用时开启）*/
    i2c_dma_config(bus->i2c_periph, I2C_DMA_OFF);
}
/*
 * * @brief  I2C DMA 初始化
 */
static void I2C_DMA_config(gd32_i2c_bus_t *bus)
{
    /*使能时钟*/
    rcu_periph_clock_enable(bus->dma_rcu);
    dma_single_data_parameter_struct dma_init_struct;
    /*配置TX DMA 参数*/
    dma_deinit(bus->dma_periph, bus->dma_tx_channel);
    dma_single_data_para_struct_init(&dma_init_struct);

    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.memory0_addr = (uint32_t)NULL;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = 0;
    dma_init_struct.periph_addr = (uint32_t)&I2C_DATA(bus->i2c_periph);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;

    dma_single_data_mode_init(bus->dma_periph, bus->dma_tx_channel, &dma_init_struct);
    dma_circulation_disable(bus->dma_periph, bus->dma_tx_channel);
    dma_channel_subperipheral_select(bus->dma_periph, bus->dma_tx_channel, bus->dma_tx_subperiph);

    /*配置RX DMA 参数*/
    dma_deinit(bus->dma_periph, bus->dma_rx_channel);

    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.memory0_addr = (uint32_t)NULL;;
    dma_init_struct.number = 0;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;

    dma_single_data_mode_init(bus->dma_periph,  bus->dma_rx_channel, &dma_init_struct);
    dma_circulation_disable(bus->dma_periph, bus->dma_rx_channel);
    dma_channel_subperipheral_select(bus->dma_periph, bus->dma_rx_channel, bus->dma_tx_subperiph);

    /*使能DMA中断*/
    nvic_irq_enable(bus->dma_tx_irq, 4, 0);
    nvic_irq_enable(bus->dma_rx_irq, 4, 0);
    /*使能发送完成中断*/
    dma_interrupt_enable(bus->dma_periph, bus->dma_tx_channel, DMA_INT_FTF);
    dma_interrupt_enable(bus->dma_periph, bus->dma_rx_channel, DMA_INT_FTF);
    /*禁止DMA（使用时开启）*/
    dma_channel_disable(bus->dma_periph, bus->dma_tx_channel);
    dma_channel_disable(bus->dma_periph, bus->dma_rx_channel);
}
/**
 * * @brief  实现数据传输操作(收发)
 */
static rt_ssize_t gd32_i2c_xfer(struct rt_i2c_bus_device *bus, struct rt_i2c_msg msgs[], rt_uint32_t num)
{
    rt_ssize_t result  = RT_EOK;
    /*rt_i2c_bus_device为gd32_i2c_bus_t第一个成员，可以强制转化*/
    gd32_i2c_bus_t *gd32_bus  = (gd32_i2c_bus_t *)bus;
    struct rt_i2c_msg *msg = NULL;
    uint32_t direction;

    for (uint8_t i = 0; i < num; i++)
    {
        msg = &msgs[i];
        direction = (msg->flags  ==  RT_I2C_WR)? I2C_TRANSMITTER:I2C_RECEIVER;
        /*检查IIC是否繁忙 */
        WAIT_FLAG_WITH_TIMEOUT(i2c_flag_get(gd32_bus->i2c_periph, I2C_FLAG_I2CBSY), result);
        if (result != RT_EOK)
        {
            LOG_W("I2C busy");
            return result;
        }
        /*发送start信号*/
        i2c_start_on_bus(gd32_bus->i2c_periph);
        /*等待start信号发送完成*/
        WAIT_FLAG_WITH_TIMEOUT(!i2c_flag_get(gd32_bus->i2c_periph, I2C_FLAG_SBSEND), result);
        if (result != RT_EOK)
        {
            LOG_W("I2C start failed");
            i2c_stop_on_bus(gd32_bus->i2c_periph);
            return result;
        }
        /*发送地址*/
        i2c_master_addressing(gd32_bus->i2c_periph, msg->addr << 1, direction);
        WAIT_FLAG_WITH_TIMEOUT(!i2c_flag_get(gd32_bus->i2c_periph, I2C_FLAG_ADDSEND), result);
        if (result != RT_EOK)
        {
            LOG_W("I2C address failed");
            i2c_stop_on_bus(gd32_bus->i2c_periph);
            return result;
        }
        i2c_flag_clear(gd32_bus->i2c_periph, I2C_FLAG_ADDSEND);
        /*配置写数据的DMA*/
        if (direction == I2C_TRANSMITTER)
        {
            /*无特别配置 DMA使用单缓存 只使用DMA_MEMORY_0*/
            dma_memory_address_config(gd32_bus->dma_periph, gd32_bus->dma_tx_channel, DMA_MEMORY_0, (uint32_t)msg->buf);
            dma_transfer_number_config(gd32_bus->dma_periph, gd32_bus->dma_tx_channel, msg->len);
            /*使能DMA传输*/
            i2c_dma_config(gd32_bus->i2c_periph, I2C_DMA_ON);
            dma_channel_enable(gd32_bus->dma_periph, gd32_bus->dma_tx_channel);
            /*自动结束*/
            //i2c_dma_last_transfer_config(gd32_bus->i2c_periph, I2C_DMALST_ON);
            /*等待发送完成*/
            if (rt_completion_wait(&gd32_bus->tx_comp, RT_TICK_PER_SECOND / 100) != RT_EOK)
            {
                LOG_E("DMA TX timeout");
                i2c_stop_on_bus(gd32_bus->i2c_periph);
            }
            LOG_D("DMA TX success"); 
            /*手动拉stop信号*/
            // i2c_stop_on_bus(gd32_bus->i2c_periph); 
            // WAIT_FLAG_WITH_TIMEOUT((I2C_CTL0(gd32_bus->i2c_periph) & I2C_CTL0_STOP), result);
            // if (result != RT_EOK)
            // {
            //     LOG_W("stop signal failed");

            //     return result;
            // }
            //  LOG_D("stop signal success"); 
        }
        else{
            dma_memory_address_config(gd32_bus->dma_periph, gd32_bus->dma_rx_channel, DMA_MEMORY_0, (uint32_t)msg->buf);
            dma_transfer_number_config(gd32_bus->dma_periph, gd32_bus->dma_rx_channel, msg->len);
            /*使能DMA传输*/
            i2c_dma_config(gd32_bus->i2c_periph, I2C_DMA_ON);
            dma_channel_enable(gd32_bus->dma_periph, gd32_bus->dma_rx_channel);
            /*自动结束*/
            i2c_dma_last_transfer_config(gd32_bus->i2c_periph, I2C_DMALST_ON);
             /*等待发送完成*/
            if (rt_completion_wait(&gd32_bus->rx_comp, RT_TICK_PER_SECOND / 100) != RT_EOK);
            {
                LOG_E("DMA RX timeout");
                i2c_stop_on_bus(gd32_bus->i2c_periph);
            }
            LOG_D("DMA RX success");  
        }
        result = i+1;
    }
    return result;
    
}

/*定义设备的操作方法：IIC做主*/
static const struct rt_i2c_bus_device_ops gd32_i2c_ops = 
{
    .master_xfer = gd32_i2c_xfer,
    .slave_xfer = RT_NULL,
    .i2c_bus_control = RT_NULL,
};

/**
 * * @brief  初始化i2c 并注册设备
 */
int rt_hw_i2c_init(void)
{
    for (int i = 0; i < sizeof(i2c_buses)/sizeof(i2c_buses[0]); i++)
    {
        gd32_i2c_bus_t *bus = &i2c_buses[i];
        /*初始化I2C外设*/
        gd32_i2c_gpio_config(bus);
        gd32_i2c_config(bus);
        I2C_DMA_config(bus);

        /*初始化发送完成信号量*/
        rt_completion_init(&bus->tx_comp);
        rt_completion_init(&bus->rx_comp);

        /*绑定设备的操作*/
        bus->i2c_bus.ops = &gd32_i2c_ops;
        /*注册I2C总线*/
        rt_i2c_bus_device_register(&bus->i2c_bus, bus->bus_name);
    }
    return RT_EOK;
}


/**
 * @brief  I2C DMA TX 中断处理函数
 */
static void gd32_i2c_dma_tx_iqr(gd32_i2c_bus_t *gd32_bus)
{
    if(dma_interrupt_flag_get(gd32_bus->dma_periph, gd32_bus->dma_tx_channel, DMA_INT_FLAG_FTF))
    {
        dma_channel_disable(gd32_bus->dma_periph, gd32_bus->dma_tx_channel);
        i2c_dma_config(gd32_bus->i2c_periph, I2C_DMA_OFF);
        dma_interrupt_flag_clear(gd32_bus->dma_periph, gd32_bus->dma_tx_channel, DMA_INT_FLAG_FTF);
        /*发送完成标志*/
        i2c_stop_on_bus(gd32_bus->i2c_periph); 
        rt_completion_done(&gd32_bus->tx_comp);
    }
}
/**
 * @brief  I2C DMA RX 中断处理函数
 */
static void gd32_i2c_dma_rx_iqr(gd32_i2c_bus_t *gd32_bus)
{
    if(dma_interrupt_flag_get(gd32_bus->dma_periph, gd32_bus->dma_rx_channel, DMA_INT_FLAG_FTF))
    {
        dma_channel_disable(gd32_bus->dma_periph, gd32_bus->dma_rx_channel);
        i2c_dma_config(gd32_bus->i2c_periph, I2C_DMA_OFF);
        dma_interrupt_flag_clear(gd32_bus->dma_periph, gd32_bus->dma_rx_channel, DMA_INT_FLAG_FTF);
        /*发送完成标志*/
        rt_completion_done(&gd32_bus->rx_comp);
    }
}

#ifdef BSP_USING_I2C0
/**
 * @brief      IIC0 DMA TX中断
 */
void DMA0_Channel6_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    gd32_i2c_dma_tx_iqr(&i2c_buses[0]);

    /* leave interrupt */
    rt_interrupt_leave();
}


/**
 * @brief      IIC0 DMA接收中断
 */
void DMA0_Channel5_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    gd32_i2c_dma_rx_iqr(&i2c_buses[0]);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#ifdef BSP_USING_I2C1
/**
 * @brief      IIC0 DMA TX中断
 */
void DMA0_Channel7_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    gd32_i2c_dma_tx_iqr(&i2c_buses[0]);

    /* leave interrupt */
    rt_interrupt_leave();
}
/**
 * @brief      IIC0 DMA接收中断
 */
void DMA0_Channel3_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    gd32_i2c_dma_rx_iqr(&i2c_buses[0]);
    
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif
#ifdef BSP_USING_I2C2
/**
 * @brief      IIC0 DMA TX中断
 */
void DMA0_Channel4_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    gd32_i2c_dma_tx_iqr(&i2c_buses[0]);

    /* leave interrupt */
    rt_interrupt_leave();
}


/**
 * @brief      IIC0 DMA接收中断
 */
void DMA0_Channel2_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    gd32_i2c_dma_rx_iqr(&i2c_buses[0]);
    
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif



/* 注册I2C设备 */
INIT_DEVICE_EXPORT(rt_hw_i2c_init);

