#ifndef LZ4_COMPRESSOR_H
#define LZ4_COMPRESSOR_H

#include <stdexcept>

#include "lz4frame.h"
#include "array.h"

class LZ4Compressor {
private:
    LZ4F_compressionContext_t compression_context;
    size_t rc;
  Array<char> compress_frame;
    size_t written_bytes = 0;

public:
  LZ4Compressor(int block_size);

    ~LZ4Compressor();

    void addValue(double v);

    void addValue32(float v);

    void close();

    Array<char> getBytes();

    long getCompressedSizeInBits();
};

#endif // LZ4_COMPRESSOR_H
