#include "chimp_decompressor.h"

ChimpDecompressor::ChimpDecompressor(const Array<uint8_t> &bs, int previousValues) {
    input_bit_stream_ = std::make_unique<InputBitStream>();
    input_bit_stream_->SetBuffer(bs);
    previousValues_ = previousValues;
    previousValuesLog2_ = (int) (std::log(previousValues_) / std::log(2));
    initialFill_ = previousValuesLog2_ + 9;
    storedValues_ = Array<uint64_t>(previousValues);
}

std::vector<double> ChimpDecompressor::decompress() {
    std::vector<double> values;
    double cur_value;
    while (!std::isnan(cur_value = nextValue())) {
        values.emplace_back(cur_value);
    }
    return values;
}

double ChimpDecompressor::nextValue() {
    if (first_) {
        first_ = false;
        stored_val_ = input_bit_stream_->ReadLong(64);
        storedValues_[current_] = stored_val_;
    } else {
        int flag = input_bit_stream_->ReadInt(2);
        uint64_t value;
        if (flag == 3) {
            storedLeadingZeros_ = leadingRep_[input_bit_stream_->ReadInt(3)];
            value = input_bit_stream_->ReadLong(64 - storedLeadingZeros_);
            value = stored_val_ ^ value;
            stored_val_ = value;
            current_ = (current_ + 1) % previousValues_;
            storedValues_[current_] = stored_val_;
        } else if (flag == 2) {
            value = input_bit_stream_->ReadLong(64 - storedLeadingZeros_);
            value = stored_val_ ^ value;
            stored_val_ = value;
            current_ = (current_ + 1) % previousValues_;
            storedValues_[current_] = stored_val_;
        } else if (flag == 1) {
            int fill = initialFill_;
            int temp = input_bit_stream_->ReadInt(fill);
            int index = temp >> (fill -= previousValuesLog2_) & (1 << previousValuesLog2_) - 1;
            storedLeadingZeros_ = leadingRep_[temp >> (fill -= 3) & (1 << 3) - 1];
            int significant_bits = temp >> (fill -= 6) & (1 << 6) - 1;
            stored_val_ = storedValues_[index];
            if (significant_bits == 0) {
                significant_bits = 64;
            }
            storedTrailingZeros_ = 64 - significant_bits - storedLeadingZeros_;
            value = input_bit_stream_->ReadLong(
                    64 - storedLeadingZeros_ - storedTrailingZeros_);
            value <<= storedTrailingZeros_;
            value = stored_val_ ^ value;
            stored_val_ = value;
            current_ = (current_ + 1) % previousValues_;
            storedValues_[current_] = stored_val_;
        } else {
            stored_val_ = storedValues_[(int) input_bit_stream_->ReadLong(
                    previousValuesLog2_)];
            current_ = (current_ + 1) % previousValues_;
            storedValues_[current_] = stored_val_;
        }
    }
    return Double::LongBitsToDouble(stored_val_);
}
