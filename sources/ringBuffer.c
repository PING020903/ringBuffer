#include "ringBuffer.h"
#include <string.h>
#ifdef RING_DEBUG
#include "DBG_macro.h"
#endif

#define RINGBUF_UPDATE_IDX(_idx, _depth) \
    do                                   \
    {                                    \
        (_idx)++;                        \
        if ((_idx) >= (2 * (_depth)))    \
        {                                \
            (_idx) -= (2 * (_depth));    \
        }                                \
    } while (0)

#define RINGBUF_ARG_CHECK(_rb)      \
    do                              \
    {                               \
        if (!(_rb))                 \
            return RINGBUF_ERR_ARG; \
        if (!((_rb)->buffer))       \
            return RINGBUF_ERR_BUF; \
    } while (0)

static inline unsigned short _calc_count(unsigned short wr, unsigned short rd, unsigned short depth)
{
    return (wr >= rd) ? (wr - rd) : (wr + depth - rd);
}

static inline void *_get_item_ptr(const ringbuf_t *rb, unsigned short idx)
{
    unsigned int actual_idx = idx % rb->depth;
    ringBuf_ptr_t ret = (ringBuf_ptr_t)rb->buffer + ((ringBuf_ptr_t)actual_idx * (ringBuf_ptr_t)rb->item_size);
    return (ret > 0) ? (void *)ret : NULL;
}

ringBuf_err_t ringBuf_clear(ringbuf_t *rb)
{
    RINGBUF_ARG_CHECK(rb);

    rb->rd_idx = 0U; // 清空读写pos
    rb->wr_idx = 0U;

    return RINGBUF_OK;
}

int ringBuf_count(const ringbuf_t *rb)
{
    if (!rb)
        return -1;
    if (!rb->buffer)
        return -2;

    return _calc_count(rb->wr_idx, rb->rd_idx, rb->depth);
}

ringBuf_err_t ringBuf_init(ringbuf_t *rb)
{
    RINGBUF_ARG_CHECK(rb);
#ifdef RING_DEBUG
    VAR_PRINT_UD(rb->item_size);
    VAR_PRINT_UD(rb->depth);
#endif

    rb->rd_idx = 0;
    rb->wr_idx = 0;
_end:
    return RINGBUF_OK;
}

ringBuf_err_t ringBuf_push(ringbuf_t *rb, const void *pData)
{
    RINGBUF_ARG_CHECK(rb);
    if (!pData)
        return RINGBUF_ERR_ARG;

    const int count = ringBuf_count(rb);
    if (count < 0)
    {
        return RINGBUF_ERR_FAIL;
    }
    if (count >= rb->depth)
    {
        if (!rb->overwritable)
        {
            return RINGBUF_ERR_WR_DENIED;
        }

        // 移动读指针, 丢弃最旧的数据
        RINGBUF_UPDATE_IDX(rb->rd_idx, rb->depth);
    }

    void *write_pos = _get_item_ptr(rb, rb->wr_idx);
    if (!write_pos)
    {
        return RINGBUF_ERR_INVALID_PTR;
    }

    memcpy(write_pos, pData, rb->item_size);
    RINGBUF_UPDATE_IDX(rb->wr_idx, rb->depth);
    return RINGBUF_OK;
}

ringBuf_err_t ringBuf_pop(ringbuf_t *rb, void *pData)
{
    RINGBUF_ARG_CHECK(rb);
    if (!pData)
        return RINGBUF_ERR_ARG;

    const int count = ringBuf_count(rb);
    if (count == 0)
    {
        return RINGBUF_ERR_EMPTY;
    }

    if (count < 0)
    {
        return RINGBUF_ERR_FAIL;
    }

    void *read_pos = _get_item_ptr(rb, rb->rd_idx);
    memcpy(pData, read_pos, rb->item_size);
    RINGBUF_UPDATE_IDX(rb->rd_idx, rb->depth);
    return RINGBUF_OK;
}

ringBuf_err_t ringBuf_peek(const ringbuf_t *rb, void *pData, const short itemIdx)
{
    RINGBUF_ARG_CHECK(rb);
    if (!pData || itemIdx < 0)
        return RINGBUF_ERR_ARG;

    const int count = ringBuf_count(rb);
    if (count == 0)
    {
        return RINGBUF_ERR_EMPTY;
    }
    if (count < 0)
    {
        return RINGBUF_ERR_FAIL;
    }

    if (itemIdx >= count)
    {
        return RINGBUF_ERR_IDX;
    }

    unsigned short target_index = rb->rd_idx + itemIdx;
    if (target_index >= 2 * rb->depth)
    {
        target_index -= 2 * rb->depth;
    }

    void *read_pos = _get_item_ptr(rb, target_index);
    memcpy(pData, read_pos, rb->item_size);
    return RINGBUF_OK;
}

ringBuf_err_t ringBuf_push_multi(ringbuf_t *rb, const void *pData, const short dataCount, short *pCount)
{
    if (!rb || !pData || dataCount < 0)
        return RINGBUF_ERR_ARG;

    const unsigned char *src = pData;
    short written = 0U;
    ringBuf_err_t err = RINGBUF_OK;

    for (written = 0U; written < dataCount; written++)
    {
        err = ringBuf_push(rb, &src[written * rb->item_size]);
        if (err)
            break;
    }

    if (pCount)
        *pCount = written;
    return err;
}

ringBuf_err_t ringBuf_pop_multi(ringbuf_t *rb, void *pData, const short dataCount, short *pCount)
{
    if (!rb || !pData || dataCount < 0)
        return RINGBUF_ERR_ARG;

    unsigned char *src = pData;
    short read_count = 0U;
    ringBuf_err_t err = RINGBUF_OK;

    for (read_count = 0U; read_count < dataCount; read_count++)
    {
        err = ringBuf_pop(rb, &src[read_count * rb->item_size]);
        if (err)
            break;
    }

    if (pCount)
        *pCount = read_count;
    return err;
}

ringBuf_err_t ringBuf_peek_multi(const ringbuf_t *rb, void *pData, const short dataCount, const short itemIdx, short *pCount)
{
    if (!rb || !pData || dataCount < 0)
        return RINGBUF_ERR_ARG;

    unsigned char *src = pData;
    short read_count = 0U, target_index = 0U;
    ringBuf_err_t err = RINGBUF_OK;

    for (read_count = 0U; read_count < dataCount; read_count++)
    {
        target_index = itemIdx + read_count;
        err = ringBuf_peek(rb, &src[read_count * rb->item_size], target_index);
        if (err)
            break;
    }

    if (pCount)
        *pCount = read_count;
    return err;
}
