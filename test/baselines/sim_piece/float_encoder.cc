#include "float_encoder.h"

void FloatEncoder::write(float number, std::ostringstream &output_stream) {
  int int_bits = Float::FloatToIntBits(number);
  IntEncoder::write(int_bits, output_stream);
}

float FloatEncoder::read(std::istringstream &input_stream) {
  int number = IntEncoder::read(input_stream);
  return Float::IntBitsToFloat(number);
}
