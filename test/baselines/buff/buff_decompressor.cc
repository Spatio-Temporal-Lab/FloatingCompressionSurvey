#include <sstream>
#include <iomanip>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include "buff_decompressor.h"

BuffDecompressor::BuffDecompressor(Array<uint8_t> bs) {
    input_bit_stream_ = std::make_unique<InputBitStream>();
    input_bit_stream_->SetBuffer(bs);
}

int BuffDecompressor::getWidthNeeded(uint64_t number) {
    if (number == 0) return 0;
    int bit_count = 0;
    while (number > 0) {
        bit_count++;
        number = number >> 1;
    }
    return bit_count;
}

Array<double> BuffDecompressor::decompress() {
    lower_bound_ = input_bit_stream_->ReadLong(64);
    batch_size_ = input_bit_stream_->ReadInt(32);
    max_prec_ = input_bit_stream_->ReadInt(32);
    int_width_ = input_bit_stream_->ReadInt(32);
    dec_width_ = precision_map_[max_prec_];
    whole_width_ = dec_width_ + int_width_ + 1;
    if (whole_width_ >= 64) {
        Array<double> result(batch_size_);
        for (auto &item: result) {
            item = Double::LongBitsToDouble(input_bit_stream_->ReadLong(64));
        }
        return result;
    }
    column_count_ = whole_width_ / 8;
    if (whole_width_ % 8 != 0) {
        column_count_++;
    }
    cols_ = Array<Array<uint8_t>>(column_count_);
    for (auto &item: cols_) item = Array<uint8_t>(batch_size_);
    sparseDecode();
    return mergeDoubles();
}

SparseResult BuffDecompressor::deserialize() {
    SparseResult result(batch_size_);
    result.set_frequent_value(input_bit_stream_->ReadInt(8));
    for (int i = 0; i < batch_size_ / 8; ++i) {
        result.bitmap_[i] = input_bit_stream_->ReadInt(8);
    }
    result.bitmap_[batch_size_ / 8] = input_bit_stream_->ReadInt(
            batch_size_ % 8);
    int count = 0;
    for (const auto &b: result.bitmap_) {
        for (int i = 0; i < 8; ++i) {
            count += (b >> i) & 1;
        }
    }
    for (int i = 0; i < count; ++i) {
        result.get_outliers()[i] = (uint8_t) input_bit_stream_->ReadInt(8);
    }

    return result;
}

void BuffDecompressor::sparseDecode() {
    for (int i = 0; i < column_count_; ++i) {
        if (input_bit_stream_->ReadBit() == 0) {
            for (int j = 0; j < batch_size_; ++j) {
                cols_[i][j] = input_bit_stream_->ReadInt(8);
            }
        } else {
            SparseResult result = deserialize();
            int index, offset, vec_count = 0;
            for (int j = 0; j < batch_size_; ++j) {
                index = j / 8;
                offset = j % 8;
                if ((result.bitmap_[index] & (1 << (7 - offset))) == 0) {
                    cols_[i][j] = result.frequent_value_;
                } else {
                    cols_[i][j] = result.outliers_[vec_count++];
                }
            }
        }
    }
}

Array<double> BuffDecompressor::mergeDoubles() {
    Array<double> dbs(batch_size_);
    for (int i = 0; i < batch_size_; ++i) {
        uint64_t bit_pack = 0;
        int remain = whole_width_ % 8;
        if (remain == 0) {
            for (int j = 0; j < column_count_; ++j) {
                bit_pack = (bit_pack << 8) | (cols_[j][i] & last_mask_[7]);
            }
        } else {
            for (int j = 0; j < column_count_ - 1; ++j) {
                bit_pack = (bit_pack << 8) | (cols_[j][i] & last_mask_[7]);
            }
            bit_pack = (bit_pack << remain) | (cols_[column_count_ - 1][i] & last_mask_[remain - 1]);
        }

        int64_t offset = (int_width_ != 0) ? (bit_pack << 65 - whole_width_ >> 64 - int_width_) : 0;
        int64_t integer = lower_bound_ + offset;
        uint64_t decimal = bit_pack << (64 - dec_width_) >> (64 - dec_width_);
        uint64_t modified_decimal = decimal << (dec_width_ - getWidthNeeded(decimal));
        int64_t exp = integer != 0 ? (getWidthNeeded(std::abs(integer)) + 1022) : 1023 - (dec_width_ - getWidthNeeded(decimal) + 1);
        int64_t exp_value = exp - 1023;
        int tmp = 53 - dec_width_ - getWidthNeeded(std::abs(integer));

        long implicit_mantissa = (std::abs(integer) << tmp + dec_width_)
                | (exp_value < 0 ? (tmp >= 0 ? (modified_decimal << tmp) : (modified_decimal >> std::abs(tmp)))
                : tmp >= 0
                ? (decimal << (tmp))
                : (decimal >> std::abs(tmp)));

        uint64_t mantissa = implicit_mantissa & 0x000FFFFFFFFFFFFFL;
        long sign = bit_pack >> (whole_width_ - 1);
        long bits = (sign << 63) | (exp << 52) | mantissa;
        double db = Double::LongBitsToDouble(bits);

        double round_num = 0.5;
        for (int j = 0; j < max_prec_; j++) {
          round_num /= 10;
        }
        db += round_num;
        std::ostringstream convert_stream;
        convert_stream << std::fixed << std::setprecision(max_prec_) << db;
        db = std::stod(convert_stream.str());

        if (db == 0 && sign == 1) db = -db;
        dbs[i] = db;
    }
    return dbs;
}
