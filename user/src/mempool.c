# include "mempool.h"

/**
 * @brief  初始化内存池
 *
 * @param  mp 内存池对象.
 *
 * @param  start 内存池的起始地址.
 *
 * @param  size 内存池大小.
 *
 * @param  block_size 内存块的大小.
 *
 * @return RT_EOK
 */
int8_t mp_init(struct mempool *mp,
                    void              *start,
                    uint32_t          size,
                    uint32_t          block_size)
{
    uint8_t *block_ptr;
    uint32_t offset;

    /* 参数检查 */
    if(mp == NULL || start == NULL || size == 0 || block_size == 0)
    {
        return -1;
    }
    
    /* 初始化内存池  未实现对齐*/
    mp->start_address = start;
    mp->size = size;

    mp->block_size = block_size;

    mp->block_total_count = mp->size / (mp->block_size + sizeof(uint8_t *));
    mp->block_free_count  = mp->block_total_count;

    /*初始化空闲内存块链表*/
    block_ptr = (uint8_t *)mp->start_address;
    for (offset = 0; offset < mp->block_total_count; offset ++)
    {
        *(uint8_t **)(block_ptr + offset * (block_size + sizeof(uint8_t *))) =
            (uint8_t *)(block_ptr + (offset + 1) * (block_size + sizeof(uint8_t *)));
    }

    *(uint8_t **)(block_ptr + (offset - 1) * (block_size + sizeof(uint8_t *))) = NULL;

    mp->block_list = block_ptr;

    return 0;
}


/**
 * @brief       从内存池申请内存块
 * @param mp    内存池对象
 *
 * @param time is the maximum waiting time for allocating memory.
 *             - 0 for not waiting, allocating memory immediately.
 *
 * @return the allocated memory block or RT_NULL on allocated failed.
 */
void *mp_alloc(mp_t mp)
{
    uint8_t *block_ptr;

    /*无空闲的内存块*/
    if(mp->block_free_count == 0)
    {
        return NULL;
    }

    /* 有可分配的内存块，空闲内存块减一 */
    mp->block_free_count--;

    /* 获取空闲内存块的地址 */
    block_ptr = mp->block_list;
    if(block_ptr == NULL)
    {
        return NULL;
    }
    /* 将空闲链表移动到下一个节点 */
    mp->block_list = *(uint8_t **)block_ptr;

    /* 将指针指向内存池，方便释放能找得到 */
    *(uint8_t **)block_ptr = (uint8_t *)mp;
    
    return (uint8_t *)(block_ptr + sizeof(uint8_t *));
}


/**
 * @brief 释放内存块
 *
 * @param block  内存块地址
 */
void mp_free(void *block)
{
    uint8_t **block_ptr;
    struct mempool *mp;

    /* 参数检查 */
    if (block == NULL) return;

    /* 获得该内存块所属的内存池对象 */
    block_ptr = (uint8_t **)((uint8_t *)block - sizeof(uint8_t *));
    mp        = (struct mempool *)*block_ptr;

    /* 空闲内存块加一 */
    mp->block_free_count ++;

    /* 将该内存块插入空闲链表 */
    *block_ptr = mp->block_list;
    mp->block_list = (uint8_t *)block_ptr;
}