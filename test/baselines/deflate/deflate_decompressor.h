#ifndef DEFLATE_DECOMPRESSOR_H
#define DEFLATE_DECOMPRESSOR_H

#include <stdexcept>
#include <vector>
#include <cmath>

#include "deflate.h"
#include "array.h"

class DeflateDecompressor {
private:
    int ret;
    z_stream strm;

public:
    DeflateDecompressor();

    ~DeflateDecompressor();

    std::vector<double> decompress(const Array<unsigned char> &bs);

    std::vector<float> decompress32(const Array<unsigned char> &bs);
};

#endif //DEFLATE_DECOMPRESSOR_H
