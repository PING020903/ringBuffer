# 环形缓冲区 (Ring Buffer)

支持**不定长数据项**的轻量级C语言环形缓冲区实现，可直接存储结构体而无需序列化，适合嵌入式系统。

## ✨ 核心特性

### 不定长 Item Size（主要特点）

与传统仅支持 `uint8_t` 的环形缓冲区不同，本库通过 `item_size` 参数支持**任意大小的数据项**：

```c
// ✅ 直接存储结构体，无需序列化
typedef struct { uint32_t ts; float temp; } SensorData;

static uint8_t buf[sizeof(SensorData) * 100];
static ringbuf_t rb = RINGBUFCRTL_INIT(buf, 100, sizeof(SensorData), false);

SensorData data = {...};
ringBuf_push(&rb, &data);  // 直接使用！
```

**对比其他实现：**
- ❌ 大多数C实现：仅支持单字节，需手动序列化
- ✅ FreeRTOS Queue：支持不定长，但需动态内存
- ✅ **本库**：不定长 + 零拷贝 + 无动态内存 + <200行代码
## 其他特性

- 🚀 **高性能索引**：扩展索引范围(0~2*depth-1)，减少取模运算约50%
- 🔄 **灵活模式**：支持覆盖/非覆盖模式
- 📦 **批量操作**：提供 `push_multi` / `pop_multi` API
- 💾 **零拷贝**：使用用户提供的静态内存，无动态分配

## ⚠️ 重要提示

**本项目仅针对裸机环境实现，未实现线程安全保护。**

在RTOS或多线程环境中使用时，需外部添加互斥锁或临界区保护。

## 快速开始

```c
#include "ringBuffer.h"

// 1. 定义数据类型和缓冲区
typedef struct { uint32_t ts; float value; } DataItem;
static uint8_t buf[sizeof(DataItem) * 50];
static ringbuf_t rb = RINGBUFCRTL_INIT(buf, 50, sizeof(DataItem), false);

int main(void) {
    ringBuf_init(&rb);
    
    // 2. 写入数据
    DataItem item = {.ts = 12345, .value = 25.5f};
    ringBuf_push(&rb, &item);
    
    // 3. 读取数据
    DataItem received;
    if (ringBuf_pop(&rb, &received) == RINGBUF_OK) {
        // 处理数据
    }
    
    return 0;
}
```

## 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

或在Windows上运行 `build.bat`

## API 参考

| 函数 | 说明 |
|------|------|
| `ringBuf_init(rb)` | 初始化缓冲区 |
| `ringBuf_clear(rb)` | 清空缓冲区 |
| `ringBuf_push(rb, &data)` | 写入单个数据项 |
| `ringBuf_pop(rb, &data)` | 读取并移除 |
| `ringBuf_peek(rb, &data)` | 查看但不移除 |
| `ringBuf_push_multi(rb, data, count, &written)` | 批量写入 |
| `ringBuf_pop_multi(rb, data, count, &read)` | 批量读取 |
| `ringBuf_count(rb)` | 获取当前数据项数量 |

**返回值：** `RINGBUF_OK` 成功，其他为错误码（`RINGBUF_ERR_EMPTY`, `RINGBUF_ERR_WR_DENIED` 等）

## 技术亮点

1. **扩展索引范围**：索引范围 0~2*depth-1，取模频率降低50%
2. **零拷贝设计**：直接使用用户提供的静态内存
3. **覆盖策略**：可选覆盖模式（丢弃旧数据）或非覆盖模式（返回错误）

完整API详见 [ringBuffer.h](include/ringBuffer.h)

## 注意事项

1. 使用前必须正确初始化环形缓冲区结构
2. 确保传入的数据指针有效且具有足够空间
3. 在中断服务程序中使用需特别注意重入问题
4. 如需在多线程环境中使用，请添加适当的同步机制

## 许可证

本项目为学习用途，可根据需要自由使用和修改。