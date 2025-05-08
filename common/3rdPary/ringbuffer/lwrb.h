/**
 * \file            lwrb.h
 * \brief           LwRB - Lightweight ring buffer
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
#ifndef LWRB_HDR_H
#define LWRB_HDR_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        LWRB Lightweight ring buffer manager
 * \brief           Lightweight ring buffer manager
 * \{
 */

typedef unsigned long lwrb_sz_atomic_t;
typedef unsigned long lwrb_sz_t;

/**
 * \brief           Buffer结构体类型声明
 */
struct lwrb;


/**
 * \brief           环形缓冲区结构体
 */
typedef struct lwrb {
    uint8_t* buff;                  /*!< 指向缓冲区的指针 */
    lwrb_sz_t size;                 /*!< 缓冲区的大小 */
    lwrb_sz_atomic_t r_ptr;         /*!< 指向下一个的读指针.buffer空：r == w buffer满 w = r-1 */
    lwrb_sz_atomic_t w_ptr;         /*!< 指向下一个的写指针 */
} lwrb_t;

uint8_t lwrb_init(lwrb_t* buff, void* buffdata, lwrb_sz_t size);
void lwrb_reset(lwrb_t* buff);

/* Read/Write functions */
lwrb_sz_t lwrb_write(lwrb_t* buff, const void* data, lwrb_sz_t btw);
lwrb_sz_t lwrb_read(lwrb_t* buff, void* data, lwrb_sz_t btr);

/* Buffer size information */
lwrb_sz_t lwrb_get_free(const lwrb_t* buff);
lwrb_sz_t lwrb_get_full(const lwrb_t* buff);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWRB_HDR_H */
