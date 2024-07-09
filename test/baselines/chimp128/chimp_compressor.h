#ifndef CHIMP_COMPRESSOR_H
#define CHIMP_COMPRESSOR_H

#include <cstdint>
#include <memory>
#include <limits>
#include <cmath>

#include "output_bit_stream.h"
#include "double.h"
#include "array.h"

class ChimpCompressor {
public:
    explicit ChimpCompressor(int previousValues);

    void addValue(double v);

    void close();

    Array<uint8_t> get_compress_pack();

    long get_size();

private:
    const uint16_t leadingRep_[64] = {
            0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 1, 1, 2, 2, 2, 2,
            3, 3, 4, 4, 5, 5, 6, 6,
            7, 7, 7, 7, 7, 7, 7, 7,
            7, 7, 7, 7, 7, 7, 7, 7,
            7, 7, 7, 7, 7, 7, 7, 7,
            7, 7, 7, 7, 7, 7, 7, 7,
            7, 7, 7, 7, 7, 7, 7, 7
    };

    const uint16_t leadingRnd_[64] = {
            0, 0, 0, 0, 0, 0, 0, 0,
            8, 8, 8, 8, 12, 12, 12, 12,
            16, 16, 18, 18, 20, 20, 22, 22,
            24, 24, 24, 24, 24, 24, 24, 24,
            24, 24, 24, 24, 24, 24, 24, 24,
            24, 24, 24, 24, 24, 24, 24, 24,
            24, 24, 24, 24, 24, 24, 24, 24,
            24, 24, 24, 24, 24, 24, 24, 24,
    };

    std::unique_ptr<OutputBitStream> output_bit_stream_;

    int storedLeadingZeros_ = std::numeric_limits<int>::max();

    int index_ = 0;

    int current_ = 0;

    long size_ = 0;

    int previousValues_;

    int previousValuesLog2_;

    int threshold_;

    int setLsb_;

    std::unique_ptr<int []> indices_;

    std::unique_ptr<uint64_t []> storedValues_;

    int flagZeroSize_;

    int flagOneSize_;

    bool first_ = true;

    Array<uint8_t> compress_pack_ = Array<uint8_t>(0);
};

#endif // CHIMP_COMPRESSOR_H