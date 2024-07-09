#ifndef CHIMP128_CHIMP_DECOMPRESSOR_32_H_
#define CHIMP128_CHIMP_DECOMPRESSOR_32_H_

#include <memory>
#include <limits>
#include <cstdint>
#include <vector>

#include "array.h"
#include "input_bit_stream.h"
#include "float.h"

class ChimpDecompressor32 {
 public:
  explicit ChimpDecompressor32(const Array<uint8_t> &bs, int previousValues);

  std::vector<float> decompress();

 private:
  constexpr static const int16_t leadingRep_[] = {0, 8, 12, 16, 18, 20, 22, 24};

  int storedLeadingZeros_ = std::numeric_limits<int>::max();

  int storedTrailingZeros_ = 0;

  uint32_t stored_val_ = 0;

  int previousValues_;

  int previousValuesLog2_;

  int initialFill_;

  Array<uint32_t> storedValues_ = Array<uint32_t>(0);

  std::unique_ptr<InputBitStream> input_bit_stream_;

  int current_ = 0;

  bool first_ = true;

  float nextValue();
};

#endif // CHIMP128_CHIMP_DECOMPRESSOR_32_H_
