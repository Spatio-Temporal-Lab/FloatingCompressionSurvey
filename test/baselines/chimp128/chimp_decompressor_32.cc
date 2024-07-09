#include "chimp_decompressor_32.h"

ChimpDecompressor32::ChimpDecompressor32(const Array<uint8_t> &bs, int previousValues) {
  input_bit_stream_ = std::make_unique<InputBitStream>();
  input_bit_stream_->SetBuffer(bs);
  previousValues_ = previousValues;
  previousValuesLog2_ = (int) (std::log(previousValues_) / std::log(2));
  storedValues_ = Array<uint32_t>(previousValues);
}

std::vector<float> ChimpDecompressor32::decompress() {
  std::vector<float> values;
  float cur_value;
  while (!std::isnan(cur_value = nextValue())) {
    values.emplace_back(cur_value);
  }
  return values;
}

float ChimpDecompressor32::nextValue() {
  if (first_) {
    first_ = false;
    stored_val_ = input_bit_stream_->ReadInt(32);
    storedValues_[current_] = stored_val_;
  } else {
    if (input_bit_stream_->ReadBit() == 1) {
      if (input_bit_stream_->ReadBit() == 1) {
        // New leading zeros
        storedLeadingZeros_ = leadingRep_[input_bit_stream_->ReadInt(3)];
      } else {
      }
      int significantBits = 32 - storedLeadingZeros_;
      if(significantBits == 0) {
        significantBits = 32;
      }
      int value = input_bit_stream_->ReadInt(32 - storedLeadingZeros_);
      value = stored_val_ ^ value;

      stored_val_ = value;
      current_ = (current_ + 1) % previousValues_;
      storedValues_[current_] = stored_val_;

    } else if (input_bit_stream_->ReadBit() == 1) {
      int fill = previousValuesLog2_ + 8;
      uint32_t temp = input_bit_stream_->ReadInt(fill);
      int index = temp >> (fill -= previousValuesLog2_) & (1 << previousValuesLog2_) - 1;
      storedLeadingZeros_ = leadingRep_[temp >> (fill -= 3) & (1 << 3) - 1];
      int significantBits = temp >> (fill -= 5) & (1 << 5) - 1;
      stored_val_ = storedValues_[index];
      if(significantBits == 0) {
        significantBits = 32;
      }
      storedTrailingZeros_ = 32 - significantBits - storedLeadingZeros_;
      int value = input_bit_stream_->ReadInt(32 - storedLeadingZeros_ - storedTrailingZeros_);
      value <<= storedTrailingZeros_;
      value = stored_val_ ^ value;
      stored_val_ = value;
      current_ = (current_ + 1) % previousValues_;
      storedValues_[current_] = stored_val_;
    } else {
      // else -> same value as before
      int index = input_bit_stream_->ReadInt(previousValuesLog2_);
      stored_val_ = storedValues_[index];
      current_ = (current_ + 1) % previousValues_;
      storedValues_[current_] = stored_val_;
    }
  }
  return Float::IntBitsToFloat(stored_val_);
}
