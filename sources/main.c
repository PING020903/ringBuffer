#include "DBG_macro.h"
#include "ringBuffer.h"
#include <stdlib.h>

char __DBG_string[DBG_DEFAULT_BUFFER_LEN] = {0};

static enum {
    itemSize = 6,
    itemCount = 9
};

static unsigned char rb_buff[itemSize * itemCount] = {0};
static ringbuf_t ringBufCtrl = RINGBUFCRTL_INIT(rb_buff, sizeof(rb_buff) / itemSize, itemSize, 0);

int main(void)
{
    srand(10);
    VAR_PRINT_INT(ringBuf_clear(&ringBufCtrl));
    unsigned char item[itemSize * itemCount];
    short count = 0;
    ringBuf_init(&ringBufCtrl);
    for (int i = 0; i < sizeof(item); i++)
    {
        item[i] = rand() % 0xff;
    }
    ringBuf_err_t err = ringBuf_push(&ringBufCtrl, item);
    VAR_PRINT_INT(err);
    DEBUG_PRINT("After 1st push: wr_idx=%d, rd_idx=%d\n", ringBufCtrl.wr_idx, ringBufCtrl.rd_idx);
    VAR_PRINT_ARR_HEX(rb_buff, sizeof(rb_buff));

    err = ringBuf_peek(&ringBufCtrl, item);
    VAR_PRINT_ARR_HEX(item, itemSize);

    err = ringBuf_push_multi(&ringBufCtrl, item, itemCount, &count);
    VAR_PRINT_INT(err);
    VAR_PRINT_INT(count);

    VAR_PRINT_ARR_HEX(rb_buff, sizeof(rb_buff));
    DEBUG_PRINT("wr[%u] rd[%u]", ringBufCtrl.wr_idx, ringBufCtrl.rd_idx);

    err = ringBuf_pop_multi(&ringBufCtrl, item, itemCount + 1, &count);
    VAR_PRINT_INT(err);
    VAR_PRINT_INT(count);
    DEBUG_PRINT("wr[%u] rd[%u]", ringBufCtrl.wr_idx, ringBufCtrl.rd_idx);

    return 0;
}