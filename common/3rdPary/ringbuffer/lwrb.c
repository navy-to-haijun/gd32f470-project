/**
 * \file            lwrb.c
 * \brief           Lightweight ring buffer
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwRB - Lightweight ring buffer library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v3.2.0
 */
#include "lwrb.h"

/* 选择的拷贝和设值的函数*/
#define BUF_MEMSET      memset
#define BUF_MEMCPY      memcpy

#define BUF_MIN(x, y)   ((x) < (y) ? (x) : (y))
#define BUF_MAX(x, y)   ((x) > (y) ? (x) : (y))

/* Optional atomic opeartions */
#define BUF_IS_VALID(b) ((b) != NULL && (b)->buff != NULL && (b)->size > 0)

/**
 * \brief           初始化
 * \param[in]       buff: Ring buffer结构体
 * \param[in]       buffdata: 指向缓存数据的指针
 * \param[in]       size: 缓存区的大小，可用大小为size - 1
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwrb_init(lwrb_t* buff, void* buffdata, lwrb_sz_t size) 
{
    if (buff == NULL || buffdata == NULL || size == 0) {
        return 0;
    }
    buff->size = size;
    buff->buff = buffdata;
    buff->w_ptr = 0;
    buff->r_ptr = 0;
    return 1;
}

/**
 * \brief           写数据将data数据写入buff，写入大小为btw，如缓冲区不够，禁止写入
 * \note            不检查buff date btw是否无效，请保证指针不为NULL， BTW>0          
 *
 * \param[in]       buff: Ring buffer instance
 * \param[in]       data: 到写入数据的指针
 * \param[in]       btw: 写入的大小 保证写入的数据不能比buffer的size - 1 还大
 * \return          `1` on success, `0` otherwise.
 */
lwrb_sz_t lwrb_write(lwrb_t* buff, const void* data, lwrb_sz_t btw) 
{
    lwrb_sz_t tocopy = 0, free = 0, w_ptr = 0;
    const uint8_t* d_ptr = data;
    /* 计算剩余字节数 */
    free = lwrb_get_free(buff);
    /* 不够，直接返回 */
    if (free < btw ) 
    {
        return 0;
    }
    w_ptr = buff->w_ptr;

    /* Step 1: 数据写入 保证写入的数据不越界（可能没有写完）*/
    tocopy = BUF_MIN(buff->size - w_ptr, btw);         
    BUF_MEMCPY(&buff->buff[w_ptr], d_ptr, tocopy);
    d_ptr += tocopy;
    w_ptr += tocopy;
    btw -= tocopy;

    /* Step 2: 继续写越界的部分 */
    if (btw > 0) 
    {
        BUF_MEMCPY(buff->buff, d_ptr, btw);
        w_ptr = btw;
    }

    /* Step 3: 若写的数据刚好写到buffer的末尾，将写指针移动到首部 */
    if (w_ptr >= buff->size) {
        w_ptr = 0;
    }

    /*将读指针写到buffer的结构体中，确保其他的地方的操作访问到读指针的中间变量*/
    buff->w_ptr = w_ptr;
    // LWRB_STORE(buff->w_ptr, memory_order_release);

    return 1;
}
/**
 * \brief           读数据.
 * 
 * \note            不检查buff date btw是否无效，请保证指针不为NULL， BTW>0 
 *
 * \param[in]       buff: Ring buffer instance
 * \param[out]      data: 将buffer输出到data中
 * \param[in]       btr: 读的大小
 * \return          `1` on success, `0` otherwise.
 */
lwrb_sz_t lwrb_read(lwrb_t* buff, void* data, lwrb_sz_t btr) 
{
    lwrb_sz_t tocopy = 0, full = 0, r_ptr = 0;
    uint8_t* d_ptr = data;

    /* 计算最大的可读空间 */
    full = lwrb_get_full(buff);
    if (full == 0 || (full < btr)) 
    {
        return 0;
    }
    r_ptr = buff->r_ptr;

    /* Step 1: 读数据，确保不越界（可能没有读完） */
    tocopy = BUF_MIN(buff->size - r_ptr, btr);
    BUF_MEMCPY(d_ptr, &buff->buff[r_ptr], tocopy);
    d_ptr += tocopy;
    r_ptr += tocopy;
    btr -= tocopy;

    /* Step 2: 继续读越界的部分 */
    if (btr > 0) {
        BUF_MEMCPY(d_ptr, buff->buff, btr);
        r_ptr = btr;
    }

    /* Step 3: 修正刚好读到buffer尾部的情况 */
    if (r_ptr >= buff->size) {
        r_ptr = 0;
    }

    /*将读指针写到buffer的结构体中，确保其他的地方的操作访问到写指针的中间变量*/
    buff->r_ptr = r_ptr;

    return 1;
}

/**
 * \brief           获取buffer可用的字节数
 * \param[in]       buff: Ring buffer instance
 * \return          可用的字节数
 */
lwrb_sz_t lwrb_get_free(const lwrb_t* buff) 
{
    lwrb_sz_t size = 0, w_ptr = 0, r_ptr = 0;

    w_ptr = buff->w_ptr;
    r_ptr = buff->r_ptr;

    if (w_ptr >= r_ptr) 
    {
        size = r_ptr - w_ptr + buff->size;
    } else
    {
        size = r_ptr - w_ptr;
    }

    /* 可用的为size-1 */
    return size - 1;
}

/**
 * \brief           获取buffer已用的空间
 * \param[in]       buff: Ring buffer instance
 * \return          已用的字节数
 */
lwrb_sz_t lwrb_get_full(const lwrb_t* buff) 
{
    lwrb_sz_t size = 0, w_ptr = 0, r_ptr = 0;
    w_ptr = buff->w_ptr;
    r_ptr = buff->r_ptr;

    if (w_ptr >= r_ptr) {
        size = w_ptr - r_ptr;
    } else {
        size = w_ptr - r_ptr + buff->size;
    }
    return size;
}

/**
 * \brief           复位buffer 
 * \note            线程不安全， 确保该buffer没有正在执行的读写操作
 * \param[in]       buff: Ring buffer instance
 */
void lwrb_reset(lwrb_t* buff) 
{
    if (BUF_IS_VALID(buff)) 
    {
        buff->w_ptr = 0;
        buff->r_ptr = 0;
    }
}