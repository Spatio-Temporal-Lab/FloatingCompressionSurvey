#ifndef BUFF_COMPRESSOR_H
#define BUFF_COMPRESSOR_H


#include <memory>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <vector>

#include "sparse_result.h"
#include "array.h"
#include "output_bit_stream.h"
#include "double.h"

class BuffCompressor {
public:
    explicit BuffCompressor(int batch_size, int max_prec);
    static int getWidthNeeded(uint64_t number);
    SparseResult findMajority(Array<uint8_t> nums);
    Array<uint8_t> get_out();
    void compress(const Array<double> &values);
    void wholeWidthLongCompress(Array<double> values);
    void close();
    long get_size();
    void headSample(const Array<double> &dbs);
    Array<Array<uint8_t>> encode(const Array<double> &dbs);
    void sparseEncode(Array<Array<uint8_t>> &cols);
    void serialize(SparseResult sr);

private:
    static constexpr int precision_map_[] = {0, 5, 8, 11, 15, 18, 21, 25, 28, 31, 35, 38, 50, 52, 52, 52, 64, 64, 64,
                                             64, 64, 64, 64};
    static constexpr long last_mask_[] = {0b1L, 0b11L, 0b111L, 0b1111L, 0b11111L, 0b111111L, 0b1111111L, 0b11111111L};
    int batch_size_ = 1000;
    std::unique_ptr<OutputBitStream> output_bit_stream_;
    long size_;
    long lower_bound_;
    int max_prec_ = -1;
    int dec_width_;
    int int_width_;
    int whole_width_;
    int column_count_;
};


#endif // BUFF_COMPRESSOR_H
