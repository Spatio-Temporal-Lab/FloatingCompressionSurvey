#ifndef INPUT_BIT_STREAM_H
#define INPUT_BIT_STREAM_H

#include <endian.h>

#include <cstdlib>
#include <cstring>
#include <vector>
#include <cmath>
#include <memory>

#include "array.h"

class InputBitStream {
 public:
    InputBitStream() = default;

    InputBitStream(uint8_t *raw_data, size_t size);

    uint64_t ReadLong(size_t len);

    uint32_t ReadInt(size_t len);

    uint32_t ReadBit();

    void SetBuffer(const Array<uint8_t> &new_buffer);

    void SetBuffer(const std::vector<uint8_t> &new_buffer);

 private:
    void Forward(size_t len);
    uint64_t Peek(size_t len);

    Array<uint32_t> data_;
    uint64_t buffer_ = 0;
    uint64_t cursor_ = 0;
    uint64_t bit_in_buffer_ = 0;
};

#endif  // INPUT_BIT_STREAM_H
