#ifndef BUFF_DECOMPRESSOR_H
#define BUFF_DECOMPRESSOR_H


#include <cstdint>
#include <memory>
#include <vector>

#include "input_bit_stream.h"
#include "double.h"
#include "sparse_result.h"

class BuffDecompressor {
public:
    explicit BuffDecompressor(Array<uint8_t> bs);
    static int getWidthNeeded(uint64_t number);
    Array<double> decompress();
    SparseResult deserialize();
    void sparseDecode();
    Array<double> mergeDoubles();

private:
    static constexpr int precision_map_[] = {0, 5, 8, 11, 15, 18, 21, 25, 28, 31, 35, 38, 50, 52, 52, 52, 64, 64, 64,
                                             64, 64, 64, 64};
    static constexpr uint64_t last_mask_[] = {0b1L, 0b11L, 0b111L, 0b1111L, 0b11111L, 0b111111L, 0b1111111L,
                                              0b11111111L};
    std::unique_ptr<InputBitStream> input_bit_stream_;
    int column_count_;
    long lower_bound_;
    int batch_size_;
    int max_prec_;
    int dec_width_;
    int int_width_;
    int whole_width_;
    Array<Array<uint8_t>> cols_;
};


#endif // BUFF_DECOMPRESSOR_H
