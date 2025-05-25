
#include "drv_eth.h"
#include "gd32f4xx_enet.h"



/*定义日志显示级别*/
#define DBG_TAG               "drv.eth"
#define DBG_LEVEL           DBG_LOG
#include <rtdbg.h>

gd32_eth_t gd32_eth_device;

/*申明描述符*/
/* ENET RxDMA/TxDMA descriptor */
extern enet_descriptors_struct  rxdesc_tab[ENET_RXBUF_NUM], txdesc_tab[ENET_TXBUF_NUM];

/* ENET receive buffer  */
extern uint8_t rx_buff[ENET_RXBUF_NUM][ENET_RXBUF_SIZE]; 

/* ENET transmit buffer */
extern uint8_t tx_buff[ENET_TXBUF_NUM][ENET_TXBUF_SIZE]; 

/*global transmit and receive descriptors pointers */
extern enet_descriptors_struct  *dma_current_txdesc;
extern enet_descriptors_struct  *dma_current_rxdesc;


/**
 * * @brief  enet外设gpio配置
 */
static void enet_gpio_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);


    /*RMII模式*/
    rcu_ckout0_config(RCU_CKOUT0SRC_PLLP, RCU_CKOUT0_DIV4);
    syscfg_enet_phy_interface_config(SYSCFG_ENET_PHY_RMII);

    /* PA1: ETH_RMII_REF_CLK */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_1);

    /* PA2: ETH_MDIO */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_2);

    /* PA7: ETH_RMII_CRS_DV */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_7);

    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_1);
    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_2);
    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_7);

    /* PB11: ETH_RMII_TX_EN */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_11);

    /* PB12: ETH_RMII_TXD0 */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_12);

    /* PB13: ETH_RMII_TXD1 */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_13);

    gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_11);
    gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_12);
    gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_13);

    /* PC1: ETH_MDC */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_1);

    /* PC4: ETH_RMII_RXD0 */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_4);

    /* PC5: ETH_RMII_RXD1 */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_5);

    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_1);
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_4);
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_5);

}
/**
 * * @brief  enet 配置
 */
static  rt_err_t enet_mac_dma_config(void)
{
    ErrStatus reval_state = ERROR;
    /* enable ethernet clock  */
    rcu_periph_clock_enable(RCU_ENET);
    rcu_periph_clock_enable(RCU_ENETTX);
    rcu_periph_clock_enable(RCU_ENETRX);

    /* reset ethernet on AHB bus */
    enet_deinit();
    reval_state = enet_software_reset();
    if (reval_state == ERROR)
    {
        LOG_E("enet reset failed");
        //return RT_EIO;
    }
    /*配置ENET模块的各类不常用功能*/
    /*配置DMA相关参数:启用自动刷新(丢弃)接收错误帧的功能、启用第二帧接收优化功能、使用标准的线性描述符结构*/
    // enet_initpara_config(DMA_OPTION, ENET_FLUSH_RXFRAME_ENABLE|ENET_SECONDFRAME_OPT_ENABLE|ENET_NORMAL_DESCRIPTOR);
    /*初始化ENET模块：PHY自协商、使能IP帧校验和功能、接收广播帧*/
    reval_state = enet_init(ENET_AUTO_NEGOTIATION, ENET_NO_AUTOCHECKSUM, ENET_BROADCAST_FRAMES_PASS);
    if (reval_state == ERROR)
    {
        LOG_E("enet init failed");
        return RT_EIO;
    }
    
    return RT_EOK;
}


/**
 * * @brief  ethernet设备初始化
 */
rt_err_t  gd32_eth_init(rt_device_t dev)
{
    rt_err_t tate = RT_EOK;
    /*初始化enet外设*/
    enet_gpio_config();
    if (enet_mac_dma_config() != RT_EOK)
    {
        LOG_E("enet mac dma failed");
        //return RT_EIO;
    }
    /*启用中断*/
    nvic_irq_enable(ENET_IRQn, 2, 0);
    /*使能发送/接收完成中断*/
    enet_interrupt_enable(ENET_DMA_INT_NIE | ENET_DMA_INT_RIE | ENET_DMA_INT_TIE);
    /*设置MAC地址 */
    enet_mac_address_set(ENET_MAC_ADDRESS0, gd32_eth_device.dev_addr);
    /*初始化发送描述符*/
    enet_descriptors_chain_init(ENET_DMA_TX);
    /*初始化接收描述符*/
    enet_descriptors_chain_init(ENET_DMA_RX);

    /* enable ethernet Rx interrrupt */
    for(uint8_t i=0; i<ENET_RXBUF_NUM; i++)
    { 
        enet_rx_desc_immediate_receive_complete_interrupt(&rxdesc_tab[i]);
    }
    /* enable MAC and DMA transmission and reception */
    enet_enable();
}

static rt_err_t gd32_eth_control(rt_device_t dev, int cmd, void *args)
{
    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* get mac address */
        if (args)
        {
            rt_memcpy(args, gd32_eth_device.dev_addr, 6);
        }
        else
        {
            return -RT_ERROR;
        }
        break;

    default :
        break;
    }

    return RT_EOK;
}

rt_err_t low_level_output(rt_device_t dev, struct pbuf *p)
{
    rt_err_t err;
    gd32_eth_t *gd32_eth = (gd32_eth_t *)dev;
    struct pbuf *q;
    int framelength = 0;
    uint8_t *buffer;

    /* 尝试获取信号量（等待最多100ms） */
    err = rt_sem_take(&gd32_eth->sem_tx_complete, rt_tick_from_millisecond(100));
    if (err != RT_EOK) 
    {
        return ERR_TIMEOUT; 
    }
    /* 填充DMA描述符 */
    buffer = (uint8_t *)(enet_desc_information_get(dma_current_txdesc, TXDESC_BUFFER_1_ADDR));
    for(q = p; q != NULL; q = q->next)
    { 
        rt_memcpy((uint8_t *)&buffer[framelength], q->payload, q->len);
        framelength = framelength + q->len;
    }
    /* 启动DMA传输 */
    err = ENET_NOCOPY_FRAME_TRANSMIT(framelength);
    if (err == SUCCESS)
    {
        return  ERR_OK;
    }
    else
    {
        return ERR_IF;
    }
}

static struct pbuf * low_level_input(rt_device_t dev)
{
    rt_err_t err;
    gd32_eth_t *gd32_eth = (gd32_eth_t *)dev;
    uint16_t len;
    uint8_t *buffer;
    struct pbuf *p= NULL, *q;
    uint32_t l =0;
    /* 尝试获取信号量（等待最多100ms） */
    err = rt_sem_take(&gd32_eth->sem_tx_complete, rt_tick_from_millisecond(100));
    if (err != RT_EOK) 
    {
    return p; 
    }
    /*获取帧长度和缓冲区地址*/
    len = enet_desc_information_get(dma_current_rxdesc, RXDESC_FRAME_LENGTH);
    buffer = (uint8_t *)enet_desc_information_get(dma_current_rxdesc, RXDESC_BUFFER_1_ADDR);
    /* 分配pbuf链*/
    if (len > 0)
    {
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    }
    /*拷贝数据到pbuf*/
    if (p != NULL)
    {
        for(q = p; q != NULL; q = q->next)
        {
            rt_memcpy((uint8_t *)q->payload, (u8_t*)&buffer[l], q->len);
            l = l + q->len;
        }
    }
    ENET_NOCOPY_FRAME_RECEIVE();
    return p;
}


/**
 * * @brief  注册ethernet设备
 */
int rt_gd32_eth_init(void)
{
    int state = RT_EOK;

    rt_sem_init(&gd32_eth_device.sem_tx_complete, "ethtxsem", 1, RT_IPC_FLAG_FIFO);
    rt_sem_init(&gd32_eth_device.sem_rx_indicate, "ethrxsem", 0, RT_IPC_FLAG_FIFO);
    /*设置MAC地址*/
    gd32_eth_device.dev_addr[0] = MAC_ADDR0;
    gd32_eth_device.dev_addr[1] = MAC_ADDR1;
    gd32_eth_device.dev_addr[2] = MAC_ADDR2;
    gd32_eth_device.dev_addr[3] = MAC_ADDR3;
    gd32_eth_device.dev_addr[4] = MAC_ADDR4;
    gd32_eth_device.dev_addr[5] = MAC_ADDR5;
    /*设置操作*/
    gd32_eth_device.parent.parent.init =  gd32_eth_init;
    gd32_eth_device.parent.parent.open =  NULL;
    gd32_eth_device.parent.parent.close = NULL;
    gd32_eth_device.parent.parent.read = NULL;
    gd32_eth_device.parent.parent.write = NULL;
    gd32_eth_device.parent.parent.control = gd32_eth_control;
    gd32_eth_device.parent.parent.user_data = NULL;

    /*设置读写接口*/
    gd32_eth_device.parent.eth_rx = low_level_input;
    gd32_eth_device.parent.eth_tx = low_level_output;
   
    /*注册以太网设备*/
    state = eth_device_init(&(gd32_eth_device.parent), "e0");
    if (RT_EOK == state)
    {
        LOG_I("eth device init success");
    }
    else
    {
        LOG_E("eth device init faild: %d", state);

    }

    return state;
}

/*enet 中断函数 */
void ETH_IRQHandler(void) 
{
    rt_interrupt_enter();

    FlagStatus flags = enet_interrupt_flag_get(ENET);
    
    /* 处理接收中断 */
    if (flags & ENET_DMA_INT_FLAG_RS) 
    {
        /*释放信号量*/
        rt_sem_release(&gd32_eth_device.sem_rx_indicate);
        /*清除标志*/
        enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_RS_CLR);
    }
    /* 处理发送中断 */
    if (flags & ENET_DMA_INT_FLAG_TS) 
    {
        /*释放信号量*/
        rt_sem_release(&gd32_eth_device.sem_tx_complete);
        /*清除标志*/
        enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_TS_CLR);
    }
    /*清除标志*/
    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_NI_CLR);

    rt_interrupt_leave();
}

/* 注册I2C设备 */
INIT_DEVICE_EXPORT(rt_gd32_eth_init);
