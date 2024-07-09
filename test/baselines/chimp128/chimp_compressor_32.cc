#include "chimp_compressor_32.h"

ChimpCompressor32::ChimpCompressor32(int previousValues) {
  output_bit_stream_ = std::make_unique<OutputBitStream>(1000 * 4);
  size_ = 0;
  previousValues_ = previousValues;
  previousValuesLog2_ = (int) (std::log(previousValues_) / std::log(2));
  threshold_ = 5 + previousValuesLog2_;
  setLsb_ = (int) std::pow(2, threshold_ + 1) - 1;
  indices_ = std::make_unique<int []>((int) std::pow(2, threshold_ + 1));
  storedValues_ = std::make_unique<uint32_t []>(previousValues_);
}

void ChimpCompressor32::addValue(float v) {
  uint32_t value = Float::FloatToIntBits(v);
  if (first_) {
    first_ = false;
    storedValues_[current_] = value;
    size_ += output_bit_stream_->WriteInt(storedValues_[current_], 32);
    indices_[((int) value) & setLsb_] = index_;
  } else {
    int key = (int) value & setLsb_;
    uint32_t xored_value;
    int previousIndex;
    int trailingZeros = 0;
    int curIndex = indices_[key];
    if ((index_ - curIndex) < previousValues_) {
      uint32_t tempXor = value ^ storedValues_[curIndex % previousValues_];
      trailingZeros = __builtin_ctz(tempXor);
      if (trailingZeros > threshold_) {
        previousIndex = curIndex % previousValues_;
        xored_value = tempXor;
      } else {
        previousIndex = index_ % previousValues_;
        xored_value = storedValues_[previousIndex] ^ value;
        trailingZeros = __builtin_ctz(xored_value);
      }
    } else {
      previousIndex = index_ % previousValues_;
      xored_value = storedValues_[previousIndex] ^ value;
      trailingZeros = __builtin_ctz(xored_value);
    }

    if (xored_value == 0) {
      size_ += output_bit_stream_->WriteInt(0, 2);
      size_ += output_bit_stream_->WriteInt(previousIndex, previousValuesLog2_);
      storedLeadingZeros_ = 33;
    } else {
      int leadingZeros = __builtin_clz(xored_value);

      if (trailingZeros > threshold_) {
        int significantBits = 32 - leadingRnd_[leadingZeros] - trailingZeros;
        size_ += output_bit_stream_->WriteInt(
            256 * (previousValues_ + previousIndex) +
                32 * leadingRep_[leadingZeros] + significantBits,
            previousValuesLog2_ + 10);
        size_ += output_bit_stream_->WriteInt(
            xored_value >> trailingZeros, significantBits);
        storedLeadingZeros_ = 33;
      } else if (leadingRnd_[leadingZeros] == storedLeadingZeros_) {
        size_ += output_bit_stream_->WriteInt(2, 2);
        int significantBits = 32 - leadingRnd_[leadingZeros];
        size_ += output_bit_stream_->WriteInt(xored_value,
                                               significantBits);
      } else {
        storedLeadingZeros_ = leadingRnd_[leadingZeros];
        int significantBits = 32 - leadingRnd_[leadingZeros];
        size_ += output_bit_stream_->WriteInt(
            24 + leadingRep_[leadingZeros], 5);
        size_ += output_bit_stream_->WriteInt(xored_value,
                                               significantBits);
      }
    }

    current_ = (current_ + 1) % previousValues_;
    storedValues_[current_] = value;
    index_++;
    indices_[key] = index_;
  }
}

void ChimpCompressor32::close() {
  addValue(std::numeric_limits<double>::quiet_NaN());
  output_bit_stream_->Flush();
}

Array<uint8_t> ChimpCompressor32::get_compress_pack() {
  compress_pack_ = output_bit_stream_->GetBuffer(std::ceil(size_ / 8.0));
  return compress_pack_;
}

long ChimpCompressor32::get_size() {
  return size_;
}
