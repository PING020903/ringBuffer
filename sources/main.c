#include "DBG_macro.h"
#include "ringBuffer.h"
#include <stdlib.h>
#include <string.h>

char __DBG_string[DBG_DEFAULT_BUFFER_LEN] = {0};

// 测试数据结构
typedef struct {
    uint8_t id;
    uint16_t value;
} TestData;

#define TEST_ITEM_SIZE sizeof(TestData)
#define TEST_ITEM_COUNT 8

static unsigned char rb_buff[TEST_ITEM_SIZE * TEST_ITEM_COUNT] = {0};
static ringbuf_t ringBufCtrl = RINGBUFCRTL_INIT(rb_buff, TEST_ITEM_COUNT, TEST_ITEM_SIZE, false);

// 测试1: 基本 push/pop
static void test_basic_push_pop(void) {
    DEBUG_PRINT("\n=== Test 1: Basic Push/Pop ===");
    ringBuf_clear(&ringBufCtrl);
    
    TestData data = {.id = 1, .value = 100};
    ringBuf_err_t err = ringBuf_push(&ringBufCtrl, &data);
    DEBUG_PRINT("Push result: %d", err);
    ringbuf_cnt_t cnt;
    ringBuf_count(&ringBufCtrl, &cnt);
    DEBUG_PRINT("Count after push: %d", cnt);
    
    TestData received;
    err = ringBuf_pop(&ringBufCtrl, &received);
    DEBUG_PRINT("Pop result: %d", err);
    DEBUG_PRINT("Received: id=%d, value=%d", received.id, received.value);
    ringBuf_count(&ringBufCtrl, &cnt);
    DEBUG_PRINT("Count after pop: %d", cnt);
}

// 测试2: peek 带索引
static void test_peek_with_index(void) {
    DEBUG_PRINT("\n=== Test 2: Peek with Index ===");
    ringBuf_clear(&ringBufCtrl);
    
    // 写入3个元素
    for (int i = 0; i < 3; i++) {
        TestData data = {.id = (uint8_t)(i + 1), .value = (uint16_t)((i + 1) * 10)};
        ringBuf_push(&ringBufCtrl, &data);
    }
    ringbuf_cnt_t cnt;
    ringBuf_count(&ringBufCtrl, &cnt);
    DEBUG_PRINT("Pushed 3 items, count: %d", cnt);
    
    // Peek 不同位置
    for (int i = 0; i < 3; i++) {
        TestData peeked;
        ringBuf_err_t err = ringBuf_peek(&ringBufCtrl, &peeked, i);
        DEBUG_PRINT("Peek[%d]: err=%d, id=%d, value=%d", i, err, peeked.id, peeked.value);
    }
    
    // 验证 peek 不改变状态
    ringBuf_count(&ringBufCtrl, &cnt);
    DEBUG_PRINT("Count after peek: %d (should still be 3)", cnt);
    
    // 测试越界
    TestData invalid;
    ringBuf_err_t err = ringBuf_peek(&ringBufCtrl, &invalid, 5);
    DEBUG_PRINT("Peek index 5 (out of range): err=%d", err);
}

// 测试3: push_multi 和 pop_multi
static void test_multi_operations(void) {
    DEBUG_PRINT("\n=== Test 3: Multi Push/Pop ===");
    ringBuf_clear(&ringBufCtrl);
    
    // 批量写入5个元素
    TestData dataArray[5];
    for (int i = 0; i < 5; i++) {
        dataArray[i].id = (uint8_t)(i + 10);
        dataArray[i].value = (uint16_t)((i + 10) * 100);
    }
    
    ringbuf_cnt_t written = 0;
    ringBuf_err_t err = ringBuf_push_multi(&ringBufCtrl, dataArray, 5, &written);
    DEBUG_PRINT("Push multi: err=%d, written=%d", err, written);
    ringbuf_cnt_t cnt;
    ringBuf_count(&ringBufCtrl, &cnt);
    DEBUG_PRINT("Count: %d", cnt);
    
    // 批量读取
    TestData readArray[5];
    ringbuf_cnt_t readCount = 0;
    err = ringBuf_pop_multi(&ringBufCtrl, readArray, 5, &readCount);
    DEBUG_PRINT("Pop multi: err=%d, read=%d", err, readCount);
    
    for (int i = 0; i < readCount; i++) {
        DEBUG_PRINT("  [%d] id=%d, value=%d", i, readArray[i].id, readArray[i].value);
    }
}

// 测试4: peek_multi
static void test_peek_multi(void) {
    DEBUG_PRINT("\n=== Test 4: Peek Multi ===");
    ringBuf_clear(&ringBufCtrl);
    
    // 写入5个元素
    for (int i = 0; i < 5; i++) {
        TestData data = {.id = (uint8_t)(i + 1), .value = (uint16_t)(i * 50)};
        ringBuf_push(&ringBufCtrl, &data);
    }
    ringbuf_cnt_t cnt;
    ringBuf_count(&ringBufCtrl, &cnt);
    DEBUG_PRINT("Pushed 5 items, count: %d", cnt);
    
    // Peek 多个元素（从索引0开始）
    TestData peekArray[5];
    ringbuf_cnt_t peeked = 0;
    ringBuf_err_t err = ringBuf_peek_multi(&ringBufCtrl, peekArray, 5, 0, &peeked);
    DEBUG_PRINT("Peek multi from 0: err=%d, peeked=%d", err, peeked);
    
    for (int i = 0; i < peeked; i++) {
        DEBUG_PRINT("  [%d] id=%d, value=%d", i, peekArray[i].id, peekArray[i].value);
    }
    
    // 验证缓冲区未被修改
    ringBuf_count(&ringBufCtrl, &cnt);
    DEBUG_PRINT("Count after peek_multi: %d (should still be 5)", cnt);
    
    // 从中间位置 peek
    TestData peekArray2[3];
    peeked = 0;
    err = ringBuf_peek_multi(&ringBufCtrl, peekArray2, 3, 2, &peeked);
    DEBUG_PRINT("\nPeek multi from index 2: err=%d, peeked=%d", err, peeked);
    for (int i = 0; i < peeked; i++) {
        DEBUG_PRINT("  [%d] id=%d, value=%d", i, peekArray2[i].id, peekArray2[i].value);
    }
}

// 测试5: 覆盖模式
static void test_overwrite_mode(void) {
    DEBUG_PRINT("\n=== Test 5: Overwrite Mode ===");
    
    static unsigned char overwrite_buf[TEST_ITEM_SIZE * 3];
    ringbuf_t overwrite_rb = RINGBUFCRTL_INIT(overwrite_buf, 3, TEST_ITEM_SIZE, true);
    ringBuf_init(&overwrite_rb);
    
    // 写入4个元素到容量为3的缓冲区
    for (int i = 0; i < 4; i++) {
        TestData data = {.id = (uint8_t)(i + 1), .value = (uint16_t)(i * 100)};
        ringBuf_err_t err = ringBuf_push(&overwrite_rb, &data);
        DEBUG_PRINT("Push item %d: err=%d", i + 1, err);
    }
    
    ringbuf_cnt_t cnt;
    ringBuf_count(&overwrite_rb, &cnt);
    DEBUG_PRINT("Count: %d (expected 3)", cnt);
    
    // 读取验证
    for (int i = 0; i < 3; i++) {
        TestData data;
        ringBuf_pop(&overwrite_rb, &data);
        DEBUG_PRINT("Popped: id=%d, value=%d", data.id, data.value);
    }
}

// 测试6: RINGBUF_ARG_CHECK 参数校验
static void test_arg_check(void) {
    DEBUG_PRINT("\n=== Test 6: ARG_CHECK Validation ===");

    // 6.1 depth 为 0 —— init 应拒绝
    {
        static unsigned char buf[64];
        ringbuf_t rb = RINGBUFCRTL_INIT(buf, 0, 4, false);
        ringBuf_err_t err = ringBuf_init(&rb);
        DEBUG_PRINT("Init with depth=0: err=%d (expected %d)", err, RINGBUF_ERR_ARG);
    }

    // 6.2 item_size 为 0 —— init 应拒绝
    {
        static unsigned char buf[64];
        ringbuf_t rb = RINGBUFCRTL_INIT(buf, 4, 0, false);
        ringBuf_err_t err = ringBuf_init(&rb);
        DEBUG_PRINT("Init with item_size=0: err=%d (expected %d)", err, RINGBUF_ERR_ARG);
    }

    // 6.3 push 也会被 ARG_CHECK 拦截（depth=0，跳过 init）
    {
        static unsigned char buf[64];
        ringbuf_t rb = RINGBUFCRTL_INIT(buf, 0, 4, false);
        rb.wr_idx = 0;
        rb.rd_idx = 0;
        int val = 42;
        ringBuf_err_t err = ringBuf_push(&rb, &val);
        DEBUG_PRINT("Push with depth=0: err=%d (expected %d)", err, RINGBUF_ERR_ARG);
    }

    // 6.4 count 也会被 ARG_CHECK 拦截（item_size=0，跳过 init）
    {
        static unsigned char buf[64];
        ringbuf_t rb = RINGBUFCRTL_INIT(buf, 4, 0, false);
        rb.wr_idx = 0;
        rb.rd_idx = 0;
        ringbuf_cnt_t cnt;
        ringBuf_err_t err = ringBuf_count(&rb, &cnt);
        DEBUG_PRINT("Count with item_size=0: err=%d (expected %d)", err, RINGBUF_ERR_ARG);
    }

    // 6.5 超大缓冲区溢出检查
    //     将 depth 设为 ringbuf_ucnt_t 上限，验证 2*depth 不溢出 uidx_t
    {
        static unsigned char buf[128];
        ringbuf_t rb = RINGBUFCRTL_INIT(buf, 8, 1, false);
        /* 强制写入 unsigned short 最大值 */
        *(ringbuf_ucnt_t *)&rb.depth = (ringbuf_ucnt_t)-1; /* 65535 */
        ringBuf_err_t err = ringBuf_init(&rb);
        /* 当前 uidx_t=unsigned int: 2*65535=131070 < UINT_MAX/2≈2e9，通过
         * 若 uidx_t=uint16_t:      2*65535=131070 > 32767，将被拦截 */
        DEBUG_PRINT("Max depth=%u: err=%d (guard active, rejects if uidx_t too narrow)",
                    (unsigned int)rb.depth, err);
    }

    // 6.6 clear 也会被 ARG_CHECK 拦截（item_size=0，跳过 init）
    {
        static unsigned char buf[64];
        ringbuf_t rb = RINGBUFCRTL_INIT(buf, 4, 0, false);
        rb.wr_idx = 0;
        rb.rd_idx = 0;
        ringBuf_err_t err = ringBuf_clear(&rb);
        DEBUG_PRINT("Clear with item_size=0: err=%d (expected %d)", err, RINGBUF_ERR_ARG);
    }
}

int main(void)
{
    DEBUG_PRINT("========================================");
    DEBUG_PRINT("Ring Buffer API Test Suite");
    DEBUG_PRINT("========================================");
    
    srand(10);
    ringBuf_init(&ringBufCtrl);
    
    test_basic_push_pop();
    test_peek_with_index();
    test_multi_operations();
    test_peek_multi();
    test_overwrite_mode();
    test_arg_check();
    
    DEBUG_PRINT("\n========================================");
    DEBUG_PRINT("All tests completed!");
    DEBUG_PRINT("========================================");
    
    return 0;
}