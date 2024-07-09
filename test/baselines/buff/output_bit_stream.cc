#include "output_bit_stream.h"

OutputBitStream::OutputBitStream(uint32_t buffer_size) {
    data_ = Array<uint32_t>(buffer_size / 4 + 1);
    buffer_ = 0;
    cursor_ = 0;
    bit_in_buffer_ = 0;
}

uint32_t OutputBitStream::Write(uint64_t content, uint32_t len) {
    content <<= (64 - len);
    buffer_ |= (content >> bit_in_buffer_);
    bit_in_buffer_ += len;
    if (bit_in_buffer_ >= 32) {
        data_[cursor_++] = (buffer_ >> 32);
        buffer_ <<= 32;
        bit_in_buffer_ -= 32;
    }
    return len;
}

uint32_t OutputBitStream::WriteLong(uint64_t content, uint64_t len) {
    if (len == 0) return 0;
    if (len > 32) {
        Write(content >> (len - 32), 32);
        Write(content, len - 32);
        return len;
    }
    return Write(content, len);
}

uint32_t OutputBitStream::WriteInt(uint32_t content, uint32_t len) {
    return Write(static_cast<uint64_t>(content), len);
}

uint32_t OutputBitStream::WriteBit(bool bit) {
    return Write(static_cast<uint64_t>(bit), 1);
}

Array<uint8_t> OutputBitStream::GetBuffer(uint32_t len) {
    Array<uint8_t> ret(len);
    for (auto &blk : data_) blk = htobe32(blk);
    __builtin_memcpy(ret.begin(), data_.begin(), len);
    return ret;
}

void OutputBitStream::Flush() {
    if (bit_in_buffer_) {
        data_[cursor_++] = buffer_ >> 32;
        buffer_ = 0;
        bit_in_buffer_ = 0;
    }
}

void OutputBitStream::Refresh() {
    cursor_ = 0;
    bit_in_buffer_ = 0;
    buffer_ = 0;
}