#ifndef CHIMP_DECOMPRESSOR_H
#define CHIMP_DECOMPRESSOR_H

#include <memory>
#include <limits>
#include <cstdint>
#include <vector>

#include "array.h"
#include "input_bit_stream.h"
#include "double.h"

class ChimpDecompressor {
public:
    explicit ChimpDecompressor(const Array<uint8_t> &bs, int previousValues);

    std::vector<double> decompress();

private:
    constexpr static const int16_t leadingRep_[] = {0, 8, 12, 16, 18, 20, 22, 24};

    int storedLeadingZeros_ = std::numeric_limits<int>::max();

    int storedTrailingZeros_ = 0;

    uint64_t stored_val_ = 0;

    int previousValues_;

    int previousValuesLog2_;

    int initialFill_;

    Array<uint64_t> storedValues_ = Array<uint64_t>(0);

    std::unique_ptr<InputBitStream> input_bit_stream_;

    int current_ = 0;

    bool first_ = true;

    double nextValue();
};

#endif //CHIMP_DECOMPRESSOR_H