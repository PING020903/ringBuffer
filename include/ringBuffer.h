#pragma once
#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <stdint.h>
#include <stdbool.h>

#define RING_DEBUG

#ifndef MIN
#define MIN(n, m) (((n) < (m)) ? (n) : (m))
#endif

#ifndef MAX
#define MAX(n, m) (((n) < (m)) ? (m) : (n))
#endif

typedef enum
{
    RINGBUF_OK = 0,
    RINGBUF_ERR_FAIL,
    RINGBUF_ERR_ARG,
    RINGBUF_ERR_BUF,
    RINGBUF_ERR_WR_DENIED,
    RINGBUF_ERR_INVALID_PTR,
    RINGBUF_ERR_EMPTY,
} ringBuf_err_t;

typedef struct ringbuf_t
{
    void *buffer;                   /**< 数据缓冲区指针 */
    const unsigned short depth;     /**< 缓冲区深度（元素个数） 运行时不可改变*/
    const unsigned short item_size; /**< 单个元素大小（字节） 运行时不可改变*/

    volatile unsigned short wr_idx; /**< 写索引（未掩码，范围 0~2*depth-1） */
    volatile unsigned short rd_idx; /**< 读索引（未掩码，范围 0~2*depth-1） */

    bool overwritable; /**< 满时是否覆盖旧数据 */
} ringbuf_t;

typedef ptrdiff_t ringBuf_ptr_t;

#define RINGBUFCRTL_INIT(_buffer, _depth, _item_sz, _overwrite) \
    {                                                           \
        .buffer = (void *)(_buffer),                            \
        .depth = (_depth),                                      \
        .item_size = (_item_sz),                                \
        .wr_idx = 0,                                            \
        .rd_idx = 0,                                            \
        .overwritable = ((_overwrite) ? true : false),          \
    }

ringBuf_err_t ringBuf_clear(ringbuf_t *rb);

ringBuf_err_t ringBuf_init(ringbuf_t *rb);

ringBuf_err_t ringBuf_push(ringbuf_t *rb, const void *pData);

ringBuf_err_t ringBuf_pop(ringbuf_t *rb, void *pData);

ringBuf_err_t ringBuf_peek(const ringbuf_t *rb, void *pData);

ringBuf_err_t ringBuf_push_multi(ringbuf_t *rb, const void *pData, const short dataCount, short *pCount);

ringBuf_err_t ringBuf_pop_multi(ringbuf_t *rb, void *pData, const short dataCount, short *pCount);

#endif