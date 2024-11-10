#ifndef __MEMPOOOL_H__
#define __MEMPOOOL_H__

#include "gd32f4xx.h"

struct mempool
{
    void                *start_address;                     // 起始地址
    uint32_t           size;                              // 内存池的大小

    uint32_t           block_size;                        // 内存块的大小
    uint8_t          *block_list;                        // 空闲内存块链表，指向第一个空闲指针

    uint32_t           block_total_count;                 // 内存块的总数量
    uint32_t           block_free_count;                  // 空闲内存块的数量
};
typedef struct mempool *mp_t;

int8_t mp_init(struct mempool *mp,
                    void              *start,
                    uint32_t          size,
                    uint32_t          block_size);



void *mp_alloc(mp_t mp);
void  mp_free(void *block);



#endif