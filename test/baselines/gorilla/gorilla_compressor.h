#ifndef GORILLA_COMPRESSOR_H
#define GORILLA_COMPRESSOR_H


#include <cmath>
#include <limits>
#include <memory>
#include "output_bit_stream.h"
#include "array.h"
#include "double.h"

class GorillaCompressor {
public:
    explicit GorillaCompressor(int capacity);

    void addValue(double v);

    void close();

    Array<uint8_t> get_compress_pack();

    long get_compress_size_in_bits();

private:
    std::unique_ptr<OutputBitStream> output_bit_stream_;

    Array<uint8_t> compress_pack_ = Array<uint8_t>(0);

    long compress_size_in_bits_ = 0;

    bool first_ = true;

    uint64_t pr_value_ = 0;

    int pr_lead_ = std::numeric_limits<int>::max();

    int pr_trail_ = 0;
};


#endif // GORILLA_COMPRESSOR_H
