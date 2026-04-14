# Ring Buffer

A lightweight C ring buffer implementation with **variable-size item support**, allowing direct storage of structs without serialization. Ideal for embedded systems.

## ✨ Core Features

### Variable Item Size (Key Feature)

Unlike traditional ring buffers that only support `uint8_t`, this library supports **arbitrary-sized data items** through the `item_size` parameter:

```c
// ✅ Store structs directly, no serialization needed
typedef struct { uint32_t ts; float temp; } SensorData;

static uint8_t buf[sizeof(SensorData) * 100];
static ringbuf_t rb = RINGBUFCRTL_INIT(buf, 100, sizeof(SensorData), false);

SensorData data = {...};
ringBuf_push(&rb, &data);  // Use directly!
```

**Comparison with other implementations:**
- ❌ Most C implementations: Single-byte only, requires manual serialization
- ✅ FreeRTOS Queue: Variable-size support, but requires dynamic memory
- ✅ **This library**: Variable-size + zero-copy + no dynamic memory + <200 lines

## Other Features

- 🚀 **High-performance indexing**: Extended index range (0~2*depth-1), reduces modulo operations by ~50%
- 🔄 **Flexible modes**: Supports overwrite/non-overwrite modes
- 📦 **Batch operations**: Provides `push_multi` / `pop_multi` APIs
- 💾 **Zero-copy**: Uses user-provided static memory, no dynamic allocation

## ⚠️ Important Notice

**This project is implemented for bare-metal environments only, without thread-safety protection.**

When using in RTOS or multi-threaded environments, external mutex locks or critical section protection must be added.

## Quick Start

```c
#include "ringBuffer.h"

// 1. Define data type and buffer
typedef struct { uint32_t ts; float value; } DataItem;
static uint8_t buf[sizeof(DataItem) * 50];
static ringbuf_t rb = RINGBUFCRTL_INIT(buf, 50, sizeof(DataItem), false);

int main(void) {
    ringBuf_init(&rb);
    
    // 2. Write data
    DataItem item = {.ts = 12345, .value = 25.5f};
    ringBuf_push(&rb, &item);
    
    // 3. Read data
    DataItem received;
    if (ringBuf_pop(&rb, &received) == RINGBUF_OK) {
        // Process data
    }
    
    return 0;
}
```

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Or run `build.bat` on Windows.

## API Reference

| Function | Description |
|----------|-------------|
| `ringBuf_init(rb)` | Initialize the buffer |
| `ringBuf_clear(rb)` | Clear the buffer |
| `ringBuf_push(rb, &data)` | Push a single item |
| `ringBuf_pop(rb, &data)` | Pop and remove an item |
| `ringBuf_peek(rb, &data)` | Peek without removing |
| `ringBuf_push_multi(rb, data, count, &written)` | Batch push |
| `ringBuf_pop_multi(rb, data, count, &read)` | Batch pop |
| `ringBuf_count(rb)` | Get current item count |

**Return values:** `RINGBUF_OK` on success, error codes otherwise (`RINGBUF_ERR_EMPTY`, `RINGBUF_ERR_WR_DENIED`, etc.)

## Technical Highlights

1. **Extended index range**: Index range 0~2*depth-1, reduces modulo frequency by 50%
2. **Zero-copy design**: Directly uses user-provided static memory
3. **Overwrite strategy**: Optional overwrite mode (discard old data) or non-overwrite mode (return error)

See [ringBuffer.h](include/ringBuffer.h) for complete API details.

## Notes

1. The ring buffer structure must be properly initialized before use
2. Ensure data pointers are valid and have sufficient space
3. Pay special attention to reentrancy when used in interrupt service routines
4. Add appropriate synchronization mechanisms for multi-threaded environments

## License

This project is for learning purposes and can be freely used and modified as needed.
