#include "chimp_compressor.h"

ChimpCompressor::ChimpCompressor(int previousValues) {
    output_bit_stream_ = std::make_unique<OutputBitStream>(1000 * 8);
    size_ = 0;
    previousValues_ = previousValues;
    previousValuesLog2_ = (int) (std::log(previousValues_) / std::log(2));
    threshold_ = 6 + previousValuesLog2_;
    setLsb_ = (int) std::pow(2, threshold_ + 1) - 1;
    indices_ = std::make_unique<int []>((int) std::pow(2, threshold_ + 1));
    storedValues_ = std::make_unique<uint64_t []>(previousValues_);
    flagZeroSize_ = previousValuesLog2_ + 2;
    flagOneSize_ = previousValuesLog2_ + 11;
}

void ChimpCompressor::addValue(double v) {
    uint64_t value = Double::DoubleToLongBits(v);
    if (first_) {
        first_ = false;
        storedValues_[current_] = value;
        size_ += output_bit_stream_->WriteLong(storedValues_[current_], 64);
        indices_[((int) value) & setLsb_] = index_;
    } else {
        int key = (int) value & setLsb_;
        uint64_t xored_value;
        int previousIndex;
        int trailingZeros = 0;
        int curIndex = indices_[key];
        if ((index_ - curIndex) < previousValues_) {
            uint64_t tempXor = value ^ storedValues_[curIndex % previousValues_];
            trailingZeros = __builtin_ctzll(tempXor);
            if (trailingZeros > threshold_) {
                previousIndex = curIndex % previousValues_;
                xored_value = tempXor;
            } else {
                previousIndex = index_ % previousValues_;
                xored_value = storedValues_[previousIndex] ^ value;
            }
        } else {
            previousIndex = index_ % previousValues_;
            xored_value = storedValues_[previousIndex] ^ value;
        }

        if (xored_value == 0) {
            size_ += output_bit_stream_->WriteInt(previousIndex, flagZeroSize_);
            storedLeadingZeros_ = 65;
        } else {
            int leadingZeros = leadingRnd_[__builtin_clzll(xored_value)];

            if (trailingZeros > threshold_) {
                int significantBits = 64 - leadingZeros - trailingZeros;
                size_ += output_bit_stream_->WriteInt(
                        512 * (previousValues_ + previousIndex) +
                        64 * leadingRep_[leadingZeros] + significantBits,
                        flagOneSize_);
                size_ += output_bit_stream_->WriteLong(
                        xored_value >> trailingZeros, significantBits);
                storedLeadingZeros_ = 65;
            } else if (leadingZeros == storedLeadingZeros_) {
                size_ += output_bit_stream_->WriteInt(2, 2);
                int significantBits = 64 - leadingZeros;
                size_ += output_bit_stream_->WriteLong(xored_value,
                                                       significantBits);
            } else {
                storedLeadingZeros_ = leadingZeros;
                int significantBits = 64 - leadingZeros;
                size_ += output_bit_stream_->WriteInt(
                        24 + leadingRep_[leadingZeros], 5);
                size_ += output_bit_stream_->WriteLong(xored_value,
                                                       significantBits);
            }
        }

        current_ = (current_ + 1) % previousValues_;
        storedValues_[current_] = value;
        index_++;
        indices_[key] = index_;
    }
}

void ChimpCompressor::close() {
    addValue(std::numeric_limits<double>::quiet_NaN());
    output_bit_stream_->Flush();
}

long ChimpCompressor::get_size() {
    return size_;
}

Array<uint8_t> ChimpCompressor::get_compress_pack() {
    compress_pack_ = output_bit_stream_->GetBuffer(std::ceil(size_ / 8.0));
    return compress_pack_;
}
