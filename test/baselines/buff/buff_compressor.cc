#include "buff_compressor.h"

BuffCompressor::BuffCompressor(int batch_size, int max_prec) {
    batch_size_ = batch_size;
    max_prec_ = max_prec;
    output_bit_stream_ = std::make_unique<OutputBitStream>(10000);
    size_ = 0;
}

int BuffCompressor::getWidthNeeded(uint64_t number) {
    if (number == 0) return 0;
    int bit_count = 0;
    while (number > 0) {
        bit_count++;
        number = number >> 1;
    }
    return bit_count;
}

SparseResult BuffCompressor::findMajority(Array<uint8_t> nums) {
    SparseResult result(batch_size_);
    uint8_t candidate = 0;
    int count = 0;

    for (const auto &num: nums) {
        if (count == 0) {
            candidate = num;
            count = 1;
        } else if (num == candidate) {
            count++;
        } else {
            count--;
        }
    }

    count = 0;
    for (int i = 0; i < nums.length(); ++i) {
        int index = i / 8;
        result.bitmap_[index] = (uint8_t) (result.bitmap_[index] << 1);
        if (nums[i] == candidate) {
            count++;
        } else {
            result.bitmap_[index] = (uint8_t) (result.bitmap_[index] | 0b1);
            result.outliers_[result.outliers_count_++] = nums[i];
        }
        if (i + 1 == nums.length() && (i + 1) % 8 != 0) {
            // Here this line of code differs from that in Java
            result.bitmap_[index] = (uint8_t) (result.bitmap_[index] << (i % 8 + 1));
        }
    }

    if (count >= nums.length() * 0.9) {
        result.flag_ = true;
        result.frequent_value_ = candidate;
    } else {
        result.flag_ = false;
    }
    return result;
}

Array<uint8_t> BuffCompressor::get_out() {
    Array<uint8_t> out = output_bit_stream_->GetBuffer(std::ceil(size_ / 8.0));
    return out;
}

void BuffCompressor::wholeWidthLongCompress(Array<double> values) {
    for (const auto &value: values) {
        size_ += output_bit_stream_->WriteLong(Double::DoubleToLongBits(value),
                                               64);
    }
}

void BuffCompressor::close() {
    output_bit_stream_->Flush();
}

long BuffCompressor::get_size() {
    return size_;
}

void BuffCompressor::compress(const Array<double> &values) {
    headSample(values);
    Array<Array<uint8_t>> cols = encode(values);
    size_ += output_bit_stream_->WriteLong(lower_bound_, 64);
    size_ += output_bit_stream_->WriteInt(batch_size_, 32);
    size_ += output_bit_stream_->WriteInt(max_prec_, 32);
    size_ += output_bit_stream_->WriteInt(int_width_, 32);
    if (whole_width_ >= 64) {
        wholeWidthLongCompress(values);
    } else {
        sparseEncode(cols);
    }
    close();
}

void BuffCompressor::headSample(const Array<double> &dbs) {
    lower_bound_ = std::numeric_limits<long>::max();
    long upper_bound = std::numeric_limits<long>::min();
    for (const auto &db: dbs) {
        uint64_t bits = Double::DoubleToLongBits(db);
        uint64_t sign = bits >> 63;
        int64_t exp = (int64_t) (bits >> 52 & 0x7FF) - 1023;
        uint64_t mantissa = bits & 0x000FFFFFFFFFFFFFL;
        uint64_t implicit_mantissa = mantissa | (1L << 52);
        uint64_t integer = exp < 0 ? 0 : (implicit_mantissa >> (52 - exp));
        int64_t integer_value = (sign == 0) ? integer : -integer;
        if (integer_value > upper_bound) {
            upper_bound = integer_value;
        }
        if (integer_value < lower_bound_) {
            lower_bound_ = integer_value;
        }
    }

    int_width_ = getWidthNeeded(upper_bound - lower_bound_);
    dec_width_ = precision_map_[max_prec_];
    whole_width_ = int_width_ + dec_width_ + 1;
    column_count_ = whole_width_ / 8;
    if (whole_width_ % 8 != 0) {
        column_count_++;
    }
}

Array<Array<uint8_t>> BuffCompressor::encode(const Array<double> &dbs) {
    Array<Array<uint8_t>> cols(column_count_);
    for (auto &col: cols) {
        col = Array<uint8_t>(dbs.length());
    }
    int db_cnt = 0;
    for (const auto &db: dbs) {
        uint64_t bits = Double::DoubleToLongBits(db);
        uint64_t sign = bits >> 63;
        int64_t exp = (int64_t) (bits >> 52 & 0x7FF) - 1023;
        uint64_t mantissa = bits & 0x000FFFFFFFFFFFFFL;
        uint64_t implicit_mantissa = mantissa | (1L << 52);
        uint64_t decimal;
        if (exp >= 0) {
            decimal = mantissa << (12 + exp) >> (64 - dec_width_);
        } else {
            if (53 - dec_width_ >= 0) {
                decimal = implicit_mantissa >> 53 - dec_width_ >> (-exp - 1);
            } else {
                decimal = implicit_mantissa << dec_width_ - 53 >> (-exp - 1);
            }
        }
        int64_t integer = exp < 0 ? 0 : (int64_t) (implicit_mantissa >> (52 - exp));
        int64_t integer_value = (sign == 0) ? integer : -integer;
        int64_t offset = integer_value - lower_bound_;
        uint64_t bit_pack = sign << (whole_width_ - 1) | (offset << dec_width_) | decimal;
        int remain = whole_width_ % 8;
        int bytes_count = 0;
        if (remain != 0) {
            bytes_count++;
            cols[column_count_ - bytes_count][db_cnt] = (uint8_t) (bit_pack & last_mask_[remain - 1]);
            bit_pack = bit_pack >> remain;
        }
        while (bytes_count < column_count_) {
            bytes_count++;
            cols[column_count_ - bytes_count][db_cnt] = (uint8_t) (bit_pack & last_mask_[7]);
            bit_pack = bit_pack >> 8;
        }

        db_cnt++;
    }
    return cols;
}

void BuffCompressor::sparseEncode(Array<Array<uint8_t>> &cols) {
    for (int i = 0; i < column_count_; ++i) {
        SparseResult result = findMajority(cols[i]);
        if (result.flag_) {
            size_ += output_bit_stream_->WriteBit(true);
            serialize(result);
        } else {
            size_ += output_bit_stream_->WriteBit(false);
            for (int j = 0; j < batch_size_; ++j) {
                size_ += output_bit_stream_->WriteInt(cols[i][j], 8);
            }
        }
    }
}

void BuffCompressor::serialize(SparseResult sr) {
    size_ += output_bit_stream_->WriteInt(sr.frequent_value_, 8);
    for (int i = 0; i < batch_size_ / 8; ++i) {
        size_ += output_bit_stream_->WriteInt(sr.bitmap_[i], 8);
    }
    size_ += output_bit_stream_->WriteInt(sr.bitmap_[batch_size_ / 8],
                                          batch_size_ % 8);
    for (int i = 0; i < sr.outliers_count_; ++i) {
        size_ += output_bit_stream_->WriteInt(sr.outliers_[i], 8);
    }
}
