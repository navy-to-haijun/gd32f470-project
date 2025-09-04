
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

    rcu_periph_clock_enable(RCU_SYSCFG);
    /*RMII模式*/
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

    enet_deinit();
    reval_state = enet_software_reset();
    if (reval_state == ERROR)
    {
        LOG_E("enet software reset failed");
    }
    reval_state = enet_init(ENET_AUTO_NEGOTIATION, ENET_AUTOCHECKSUM_DROP_FAILFRAMES, ENET_RECEIVEALL);
    if (reval_state == ERROR)
    {
        LOG_E("enet init failed");
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
    enet_interrupt_enable(ENET_DMA_INT_NIE);
    enet_interrupt_enable(ENET_DMA_INT_RIE);
    // enet_interrupt_enable(ENET_DMA_INT_TIE);
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
    return tate;
}
/**
 * * @brief  适配操作，无实际用处
 */
static rt_err_t rt_stm32_eth_open(rt_device_t dev, rt_uint16_t oflag)
{
    LOG_D("emac open");
    return RT_EOK;
}
/**
 * * @brief  适配操作，无实际用处
 */
static rt_err_t rt_stm32_eth_close(rt_device_t dev)
{
    LOG_D("enet close");
    return RT_EOK;
}
/**
 * * @brief  适配操作，无实际用处
 */
static rt_ssize_t rt_stm32_eth_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    LOG_D("enet read");
    return RT_EOK;
}
/**
 * * @brief  适配操作，无实际用处
 */
static rt_ssize_t rt_stm32_eth_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    LOG_D("enet write");
    return RT_EOK;
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
    int max_wait = 1000;

    /* 填充DMA描述符 */
    while((uint32_t)RESET != (dma_current_txdesc->status & ENET_TDES0_DAV))
    {
        /* 等待DMA描述符可用 */
        if (--max_wait == 0)
        {
            LOG_E("TX descriptor timeout");
            return ERR_IF;
        }
    }    
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
        LOG_D("low_level_output failed: %d", err);
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
 * * @brief  phy链路状态变化处理函数(定时器时间)
 */
static void phy_linkchange(void *parameter)
{
    uint16_t phy_value = 0U;
    ErrStatus reval_state = ERROR;
    uint8_t now_link_value;
    static uint8_t last_link_value = 0xFF;

    /*读取PHY状态寄存器*/
    enet_phy_write_read(ENET_PHY_READ, PHY_ADDRESS, PHY_REG_BSR, &phy_value);
    now_link_value = phy_value & 0x04;

    if (now_link_value != last_link_value)
    {
        last_link_value = now_link_value;
        /*bit2: Link Status*/
        if (now_link_value)
        {
            LOG_I("phy link up");
            eth_device_linkchange(&(gd32_eth_device.parent), RT_TRUE);
        }
        else
        {
            LOG_I("phy link down");
            eth_device_linkchange(&(gd32_eth_device.parent), RT_FALSE);
        }
    }
}
   
/**
 * * @brief  注册ethernet设备
 */
int rt_gd32_eth_init(void)
{
    int state = RT_EOK;
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

     /*创建一个定时器，用于查询网口的链路情况：定时时间1秒 周期性、*/
    gd32_eth_device.poll_link_timer = rt_timer_create("phylnk",
                                      phy_linkchange,
                                      NULL, 
                                      RT_TICK_PER_SECOND, 
                                      RT_TIMER_FLAG_PERIODIC);

    if (gd32_eth_device.poll_link_timer == NULL || rt_timer_start(gd32_eth_device.poll_link_timer) != RT_EOK)
    {
        LOG_E("phy link timer create failed");
        state = -RT_ERROR;
    }
    LOG_I("phy link timer create success");
    /*注册以太网设备*/
    state = eth_device_init(&(gd32_eth_device.parent), "e0");
    if (state !=  RT_EOK)
    {
        LOG_E("eth device init faild: %d", state);
        return state;
        
    }
    LOG_I("eth device init success");
    return state;
}

/*enet 中断函数 */
void ENET_IRQHandler(void) 
{
    rt_interrupt_enter();
    if(SET == enet_interrupt_flag_get(ENET_DMA_INT_FLAG_RS))
    {
        /*发送邮件通知eth_rx_thread_entry*/
        eth_device_ready(&(gd32_eth_device.parent));
        /*清除标志*/
        enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_RS_CLR);
    }
    // if(SET == enet_interrupt_flag_get(ENET_DMA_INT_FLAG_TS_CLR))
    // {
    //     /*清除标志*/
    //     enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_TS_CLR);
    // }
    if (SET == enet_interrupt_flag_get(ENET_DMA_INT_FLAG_NI_CLR))
    {
        /*清除标志*/
        enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_NI_CLR);
    }
    
    

    rt_interrupt_leave();
}

/* 注册eth设备 */
INIT_DEVICE_EXPORT(rt_gd32_eth_init);
