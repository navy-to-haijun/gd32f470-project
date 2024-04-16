/**
 * \file            lwrb.c
 * \brief           Lightweight ring buffer
 */

/*
 * Copyright (c) 2023 Tilen MAJERLE
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
 * Version:         v3.1.0
 */
#include "lwrb.h"

/* Memory set and copy functions */
#define BUF_MEMSET      memset
#define BUF_MEMCPY      memcpy

#define BUF_IS_VALID(b) ((b) != NULL && (b)->buff != NULL && (b)->size > 0)
#define BUF_MIN(x, y)   ((x) < (y) ? (x) : (y))
#define BUF_MAX(x, y)   ((x) > (y) ? (x) : (y))
#define BUF_SEND_EVT(b, type, bp)                                                                                      \
    do {                                                                                                               \
        if ((b)->evt_fn != NULL) {                                                                                     \
            (b)->evt_fn((void*)(b), (type), (bp));                                                                     \
        }                                                                                                              \
    } while (0)

/* Optional atomic opeartions */
#ifdef LWRB_DISABLE_ATOMIC
#define LWRB_INIT(var, val)        (var) = (val)
#define LWRB_LOAD(var, type)       (var)
#define LWRB_STORE(var, val, type) (var) = (val)
#else
#define LWRB_INIT(var, val)        atomic_init(&(var), (val))
#define LWRB_LOAD(var, type)       atomic_load_explicit(&(var), (type))
#define LWRB_STORE(var, val, type) atomic_store_explicit(&(var), (val), (type))
#endif

/**
 * \brief           初始化环形缓冲区
 * \param[in]       buff: 缓冲区结构体指针
 * \param[in]       buffdata: 缓冲区指针
 * \param[in]       size: 缓冲区大小，实际缓冲区大小为size - 1
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwrb_init(lwrb_t* buff, void* buffdata, lwrb_sz_t size) {
    if (buff == NULL || buffdata == NULL || size == 0) {
        return 0;
    }

    buff->evt_fn = NULL;
    buff->size = size;
    buff->buff = buffdata;
    /*清零*/
    LWRB_INIT(buff->w, 0);
    LWRB_INIT(buff->r, 0);
    return 1;
}

/**
 * \brief           Check if buff is initialized and ready to use
 * \param[in]       buff: Ring buffer instance
 * \return          `1` if ready, `0` otherwise
 */
uint8_t
lwrb_is_ready(lwrb_t* buff) {
    return BUF_IS_VALID(buff);
}

/**
 * \brief           Free buffer memory
 * \note            Since implementation does not use dynamic allocation,
 *                  it just sets buffer handle to `NULL`
 * \param[in]       buff: Ring buffer instance
 */
void
lwrb_free(lwrb_t* buff) {
    if (BUF_IS_VALID(buff)) {
        buff->buff = NULL;
    }
}

/**
 * \brief           Set event function callback for different buffer operations
 * \param[in]       buff: Ring buffer instance
 * \param[in]       evt_fn: Callback function
 */
void
lwrb_set_evt_fn(lwrb_t* buff, lwrb_evt_fn evt_fn) {
    if (BUF_IS_VALID(buff)) {
        buff->evt_fn = evt_fn;
    }
}

/**
 * \brief           读数据到环形缓冲区
 *
 * \param[in]       buff: 环形缓冲区结构体指针
 * \param[in]       data: 待写入数据的指针
 * \param[in]       btw: 待写入字节数
 * \return          写入缓冲区的字节数，当写入数据 < btw,说明没有足够可用的内存
 */
lwrb_sz_t lwrb_write(lwrb_t* buff, const void* data, lwrb_sz_t btw) {
    lwrb_sz_t written = 0;

    if (lwrb_write_ex(buff, data, btw, &written, 0)) {
        return written;
    }
    return 0;
}

/**
 * \brief           真正实现写数据功能
 * 
 * \param           buff: 环形缓冲区结构体指针
 * \param           data: 待写入数据的指针
 * \param           btw: 待写入字节数
 * \param           bw: 输出写入的字节数的指针
 * \param           flags: Optional flags.1：内存不满足不写入；0：内存不满足，写入部分数据
 * 
 * \return          `1` if write operation OK, `0` otherwise
 */
uint8_t lwrb_write_ex(lwrb_t* buff, const void* data, lwrb_sz_t btw, lwrb_sz_t* bw, uint16_t flags) {
    lwrb_sz_t tocopy, free, buff_w_ptr;
    const uint8_t* d = data;

    if (!BUF_IS_VALID(buff) || data == NULL || btw == 0) {
        return 0;
    }

    /* 计算能写入的最大字节数 */
    free = lwrb_get_free(buff);
    /* 没有空间 or 待写入的数据小于剩余空间*/
    if (free == 0 || (free < btw && flags & LWRB_FLAG_WRITE_ALL)) {
        return 0;
    }
    btw = BUF_MIN(free, btw);
    buff_w_ptr = LWRB_LOAD(buff->w, memory_order_acquire);

    /* Step 1: Write data to linear part of buffer */
    tocopy = BUF_MIN(buff->size - buff_w_ptr, btw);
    BUF_MEMCPY(&buff->buff[buff_w_ptr], d, tocopy);
    buff_w_ptr += tocopy;
    btw -= tocopy;

    /* Step 2: Write data to beginning of buffer (overflow part) */
    if (btw > 0) {
        BUF_MEMCPY(buff->buff, &d[tocopy], btw);
        buff_w_ptr = btw;
    }

    /* Step 3: Check end of buffer */
    if (buff_w_ptr >= buff->size) {
        buff_w_ptr = 0;
    }

    /*
     * Write final value to the actual running variable.
     * This is to ensure no read operation can access intermediate data
     */
    LWRB_STORE(buff->w, buff_w_ptr, memory_order_release);

    BUF_SEND_EVT(buff, LWRB_EVT_WRITE, tocopy + btw);
    if (bw != NULL) {
        *bw = tocopy + btw;
    }
    return 1;
}

/**
 * \brief           读数据
 *
 * \param[in]       buff: Ring buffer instance
 * \param[out]      data: 待读入数据指针
 * \param[in]       btr: 待读的个数
 * \return          实际读取的个数
 */
lwrb_sz_t lwrb_read(lwrb_t* buff, void* data, lwrb_sz_t btr) {
    lwrb_sz_t read = 0;

    if (lwrb_read_ex(buff, data, btr, &read, 0)) {
        return read;
    }
    return 0;
}

/**
 * \brief           真正实现读数据功能
 * 
 * \param           buff: Ring buffer instance
 * \param           data: 待读入数据指针
 * \param           btr: 待读的个数
 * \param           br: 输出读入的字节数的指针
 * \param           flags: Optional flags
 *                  
 * \return          `1` if read operation OK, `0` otherwise
 */
uint8_t
lwrb_read_ex(lwrb_t* buff, void* data, lwrb_sz_t btr, lwrb_sz_t* br, uint16_t flags) {
    lwrb_sz_t tocopy, full, buff_r_ptr;
    uint8_t* d = data;

    if (!BUF_IS_VALID(buff) || data == NULL || btr == 0) {
        return 0;
    }

    /* 计算能读的最大空间 */
    full = lwrb_get_full(buff);
    /*无读空间 or 读空间不够*/
    if (full == 0 || (full < btr && (flags & LWRB_FLAG_READ_ALL))) {
        return 0;
    }
    btr = BUF_MIN(full, btr);
    buff_r_ptr = LWRB_LOAD(buff->r, memory_order_acquire);

    /* Step 1: Read data from linear part of buffer */
    tocopy = BUF_MIN(buff->size - buff_r_ptr, btr);
    BUF_MEMCPY(d, &buff->buff[buff_r_ptr], tocopy);
    buff_r_ptr += tocopy;
    btr -= tocopy;

    /* Step 2: Read data from beginning of buffer (overflow part) */
    if (btr > 0) {
        BUF_MEMCPY(&d[tocopy], buff->buff, btr);
        buff_r_ptr = btr;
    }

    /* Step 3: Check end of buffer */
    if (buff_r_ptr >= buff->size) {
        buff_r_ptr = 0;
    }

    /*
     * Write final value to the actual running variable.
     * This is to ensure no write operation can access intermediate data
     */
    LWRB_STORE(buff->r, buff_r_ptr, memory_order_release);

    BUF_SEND_EVT(buff, LWRB_EVT_READ, tocopy + btr);
    if (br != NULL) {
        *br = tocopy + btr;
    }
    return 1;
}

/**
 * \brief           读数据，不改变指针位置 (仅查看)
 * \param[in]       buff: Ring buffer instance
 * \param[in]       skip_count: Number of bytes to skip before reading data
 * \param[out]      data: Pointer to output memory to copy buffer data to
 * \param[in]       btp: Number of bytes to peek
 * \return          Number of bytes peeked and written to output array
 */
lwrb_sz_t lwrb_peek(const lwrb_t* buff, lwrb_sz_t skip_count, void* data, lwrb_sz_t btp) {
    lwrb_sz_t full, tocopy, r;
    uint8_t* d = data;

    if (!BUF_IS_VALID(buff) || data == NULL || btp == 0) {
        return 0;
    }

    /*
     * Calculate maximum number of bytes available to read
     * and check if we can even fit to it
     */
    full = lwrb_get_full(buff);
    if (skip_count >= full) {
        return 0;
    }
    r = LWRB_LOAD(buff->r, memory_order_relaxed);
    r += skip_count;
    full -= skip_count;
    if (r >= buff->size) {
        r -= buff->size;
    }

    /* Check maximum number of bytes available to read after skip */
    btp = BUF_MIN(full, btp);
    if (btp == 0) {
        return 0;
    }

    /* Step 1: Read data from linear part of buffer */
    tocopy = BUF_MIN(buff->size - r, btp);
    BUF_MEMCPY(d, &buff->buff[r], tocopy);
    btp -= tocopy;

    /* Step 2: Read data from beginning of buffer (overflow part) */
    if (btp > 0) {
        BUF_MEMCPY(&d[tocopy], buff->buff, btp);
    }
    return tocopy + btp;
}

/**
 * \brief           获取剩余空间大小
 * \param[in]       buff: 环形缓冲区结构体指针
 * \return         空闲字节数
 */
lwrb_sz_t lwrb_get_free(const lwrb_t* buff) {
    lwrb_sz_t size, w, r;

    if (!BUF_IS_VALID(buff)) {
        return 0;
    }

    /*读写指针使用局部变量，保证线程安全*/
    w = LWRB_LOAD(buff->w, memory_order_relaxed);
    r = LWRB_LOAD(buff->r, memory_order_relaxed);

    if (w == r) {
        size = buff->size;
    } else if (r > w) {
        size = r - w;
    } else {
        size = buff->size - (w - r);
    }

    /* Buffer free size is always 1 less than actual size */
    return size - 1;
}

/**
 * \brief           获取缓冲区中当前可用的字节数
 * \param[in]       buff: Ring buffer instance
 * \return          可读的最大字节数
 */
lwrb_sz_t lwrb_get_full(const lwrb_t* buff) {
    lwrb_sz_t size, w, r;

    if (!BUF_IS_VALID(buff)) {
        return 0;
    }

    w = LWRB_LOAD(buff->w, memory_order_relaxed);
    r = LWRB_LOAD(buff->r, memory_order_relaxed);

    if (w == r) {
        size = 0;
    } else if (w > r) {
        size = w - r;
    } else {
        size = buff->size - (r - w);
    }
    return size;
}

/**
 * \brief           重置环形缓冲区，缓冲大小不变
 * \note            无法保证线程安全，在使用时保证无读/写操作
 * \param[in]       buff: 环形缓冲区结构体
 */
void lwrb_reset(lwrb_t* buff) {
    if (BUF_IS_VALID(buff)) {
        LWRB_STORE(buff->w, 0, memory_order_release);
        LWRB_STORE(buff->r, 0, memory_order_release);
        BUF_SEND_EVT(buff, LWRB_EVT_RESET, 0);
    }
}

/**
 * \brief           Get linear address for buffer for fast read
 * \param[in]       buff: Ring buffer instance
 * \return          Linear buffer start address
 */
void*
lwrb_get_linear_block_read_address(const lwrb_t* buff) {
    if (!BUF_IS_VALID(buff)) {
        return NULL;
    }
    return &buff->buff[buff->r];
}

/**
 * \brief           Get length of linear block address before it overflows for read operation
 * \param[in]       buff: Ring buffer instance
 * \return          Linear buffer size in units of bytes for read operation
 */
lwrb_sz_t
lwrb_get_linear_block_read_length(const lwrb_t* buff) {
    lwrb_sz_t len, w, r;

    if (!BUF_IS_VALID(buff)) {
        return 0;
    }

    /*
     * Use temporary values in case they are changed during operations.
     * See lwrb_buff_free or lwrb_buff_full functions for more information why this is OK.
     */
    w = LWRB_LOAD(buff->w, memory_order_relaxed);
    r = LWRB_LOAD(buff->r, memory_order_relaxed);

    if (w > r) {
        len = w - r;
    } else if (r > w) {
        len = buff->size - r;
    } else {
        len = 0;
    }
    return len;
}

/**
 * \brief           Skip (ignore; advance read pointer) buffer data
 * Marks data as read in the buffer and increases free memory for up to `len` bytes
 *
 * \note            Useful at the end of streaming transfer such as DMA
 * \param[in]       buff: Ring buffer instance
 * \param[in]       len: Number of bytes to skip and mark as read
 * \return          Number of bytes skipped
 */
lwrb_sz_t lwrb_skip(lwrb_t* buff, lwrb_sz_t len) {
    lwrb_sz_t full, r;

    if (!BUF_IS_VALID(buff) || len == 0) {
        return 0;
    }

    full = lwrb_get_full(buff);
    len = BUF_MIN(len, full);
    r = LWRB_LOAD(buff->r, memory_order_acquire);
    r += len;
    if (r >= buff->size) {
        r -= buff->size;
    }
    LWRB_STORE(buff->r, r, memory_order_release);
    BUF_SEND_EVT(buff, LWRB_EVT_READ, len);
    return len;
}

/**
 * \brief           Get linear address for buffer for fast read
 * \param[in]       buff: Ring buffer instance
 * \return          Linear buffer start address
 */
void*
lwrb_get_linear_block_write_address(const lwrb_t* buff) {
    if (!BUF_IS_VALID(buff)) {
        return NULL;
    }
    return &buff->buff[buff->w];
}

/**
 * \brief           Get length of linear block address before it overflows for write operation
 * \param[in]       buff: Ring buffer instance
 * \return          Linear buffer size in units of bytes for write operation
 */
lwrb_sz_t
lwrb_get_linear_block_write_length(const lwrb_t* buff) {
    lwrb_sz_t len, w, r;

    if (!BUF_IS_VALID(buff)) {
        return 0;
    }

    /*
     * Use temporary values in case they are changed during operations.
     * See lwrb_buff_free or lwrb_buff_full functions for more information why this is OK.
     */
    w = LWRB_LOAD(buff->w, memory_order_relaxed);
    r = LWRB_LOAD(buff->r, memory_order_relaxed);

    if (w >= r) {
        len = buff->size - w;
        /*
         * When read pointer is 0,
         * maximal length is one less as if too many bytes
         * are written, buffer would be considered empty again (r == w)
         */
        if (r == 0) {
            /*
             * Cannot overflow:
             * - If r is not 0, statement does not get called
             * - buff->size cannot be 0 and if r is 0, len is greater 0
             */
            --len;
        }
    } else {
        len = r - w - 1;
    }
    return len;
}

/**
 * \brief           Advance write pointer in the buffer.
 * Similar to skip function but modifies write pointer instead of read
 *
 * \note            Useful when hardware is writing to buffer and application needs to increase number
 *                      of bytes written to buffer by hardware
 * \param[in]       buff: Ring buffer instance
 * \param[in]       len: Number of bytes to advance
 * \return          Number of bytes advanced for write operation
 */
lwrb_sz_t
lwrb_advance(lwrb_t* buff, lwrb_sz_t len) {
    lwrb_sz_t free, w;

    if (!BUF_IS_VALID(buff) || len == 0) {
        return 0;
    }

    /* Use local variables before writing back to main structure */
    free = lwrb_get_free(buff);
    len = BUF_MIN(len, free);
    w = LWRB_LOAD(buff->w, memory_order_acquire);
    w += len;
    if (w >= buff->size) {
        w -= buff->size;
    }
    LWRB_STORE(buff->w, w, memory_order_release);
    BUF_SEND_EVT(buff, LWRB_EVT_WRITE, len);
    return len;
}

/**
 * \brief           Searches for a *needle* in an array, starting from given offset.
 * 
 * \note            This function is not thread-safe. 
 * 
 * \param           buff: Ring buffer to search for needle in
 * \param           bts: Constant byte array sequence to search for in a buffer
 * \param           len: Length of the \arg bts array 
 * \param           start_offset: Start offset in the buffer
 * \param           found_idx: Pointer to variable to write index in array where bts has been found
 *                      Must not be set to `NULL`
 * \return          `1` if \arg bts found, `0` otherwise
 */
uint8_t
lwrb_find(const lwrb_t* buff, const void* bts, lwrb_sz_t len, lwrb_sz_t start_offset, lwrb_sz_t* found_idx) {
    lwrb_sz_t full, r, max_x;
    uint8_t found = 0;
    const uint8_t* needle = bts;

    if (!BUF_IS_VALID(buff) || needle == NULL || len == 0 || found_idx == NULL) {
        return 0;
    }
    *found_idx = 0;

    full = lwrb_get_full(buff);
    /* Verify initial conditions */
    if (full < (len + start_offset)) {
        return 0;
    }

    /* Max number of for loops is buff_full - input_len - start_offset of buffer length */
    max_x = full - len;
    for (lwrb_sz_t skip_x = start_offset; !found && skip_x <= max_x; ++skip_x) {
        found = 1; /* Found by default */

        /* Prepare the starting point for reading */
        r = buff->r + skip_x;
        if (r >= buff->size) {
            r -= buff->size;
        }

        /* Search in the buffer */
        for (lwrb_sz_t i = 0; i < len; ++i) {
            if (buff->buff[r] != needle[i]) {
                found = 0;
                break;
            }
            if (++r >= buff->size) {
                r = 0;
            }
        }
        if (found) {
            *found_idx = skip_x;
        }
    }
    return found;
}
