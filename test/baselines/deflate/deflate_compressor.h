#ifndef DEFLATE_COMPRESSOR_H
#define DEFLATE_COMPRESSOR_H

#include <stdexcept>

#include "deflate.h"
#include "array.h"

class DeflateCompressor {
private:
    int ret;
    z_stream strm;
  Array<unsigned char> compress_pack;

public:
  DeflateCompressor(int block_size);

    ~DeflateCompressor();

    void addValue(double v);

    void addValue32(float v);

    void close();

    Array<unsigned char> getBytes();

    long getCompressedSizeInBits();
};

#endif //DEFLATE_COMPRESSOR_H
