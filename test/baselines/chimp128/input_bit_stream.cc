#include "input_bit_stream.h"

InputBitStream::InputBitStream(uint8_t *raw_data, size_t size) {
    data_ = Array<uint32_t>(std::ceil(static_cast<double>(size) / sizeof
            (uint32_t)));
    __builtin_memcpy(data_.begin(), raw_data, size);
    for (auto &blk : data_) blk = be32toh(blk);
    buffer_ = (static_cast<uint64_t>(data_[0])) << 32;
    cursor_ = 1;
    bit_in_buffer_ = 32;
}

uint64_t InputBitStream::Peek(size_t len) {
    return buffer_ >> (64 - len);
}

void InputBitStream::Forward(size_t len) {
    bit_in_buffer_ -= len;
    buffer_ <<= len;
    if (bit_in_buffer_ < 32) {
        if (cursor_ < data_.length()) {
            auto next = (uint64_t) data_[cursor_];
            buffer_ |= (next << (32 - bit_in_buffer_));
            bit_in_buffer_ += 32;
            cursor_++;
        } else {
            bit_in_buffer_ = 64;
        }
    }
}

uint64_t InputBitStream::ReadLong(size_t len) {
    if (len == 0) return 0;
    uint64_t ret = 0;
    if (len > 32) {
        ret = Peek(32);
        Forward(32);
        ret <<= len - 32;
        len -= 32;
    }
    ret |= Peek(len);
    Forward(len);
    return ret;
}

uint32_t InputBitStream::ReadInt(size_t len) {
    if (len == 0) return 0;
    uint32_t ret = 0;
    ret |= Peek(len);
    Forward(len);
    return ret;
}

uint32_t InputBitStream::ReadBit() {
    uint32_t ret = Peek(1);
    Forward(1);
    return ret;
}

void InputBitStream::SetBuffer(const Array<uint8_t> &new_buffer) {
    data_ = Array<uint32_t>(std::ceil(static_cast<double>(new_buffer.length()) /
                                      sizeof(uint32_t)));
    __builtin_memcpy(data_.begin(), new_buffer.begin(), new_buffer.length());
    for (auto &blk : data_) blk = be32toh(blk);
    buffer_ = (static_cast<uint64_t>(data_[0])) << 32;
    cursor_ = 1;
    bit_in_buffer_ = 32;
}

void InputBitStream::SetBuffer(const std::vector<uint8_t> &new_buffer) {
    data_ = Array<uint32_t>(std::ceil(static_cast<double>(new_buffer.size()) /
                                      sizeof(uint32_t)));
    __builtin_memcpy(data_.begin(), new_buffer.data(), new_buffer.size());
    for (auto &blk : data_) blk = be32toh(blk);
    buffer_ = (static_cast<uint64_t>(data_[0])) << 32;
    cursor_ = 1;
    bit_in_buffer_ = 32;
}
