#ifndef __DRV_ETH_H__
#define __DRV_ETH_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <netif/ethernetif.h>

#define MAX_ADDR_LEN 6

#define MAC_ADDR0   2
#define MAC_ADDR1   0xA
#define MAC_ADDR2   0xF
#define MAC_ADDR3   0xE
#define MAC_ADDR4   0xD
#define MAC_ADDR5   6

typedef struct 
{
    struct eth_device   parent;                         // 父类结构体
    uint8_t             dev_addr[MAX_ADDR_LEN];         // 设备地址
    struct rt_semaphore  sem_tx_complete;               // 发送完成信号量
    struct rt_semaphore  sem_rx_indicate;               // 接收完成信号量

}gd32_eth_t;



#endif /* __DRV_ETH_H__ */