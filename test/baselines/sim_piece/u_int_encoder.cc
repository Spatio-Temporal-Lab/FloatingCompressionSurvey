#include "u_int_encoder.h"

void UIntEncoder::write(long number, std::ostringstream &output_stream) {
  int res = static_cast<int>((number & 0xFFFFFFFFL));
  output_stream.write(reinterpret_cast<char *>(&res), sizeof(res));
}

long UIntEncoder::read(std::istringstream &input_stream) {
  int ret;
  input_stream.read(reinterpret_cast<char *>(&ret), sizeof(ret));
//  char *byte_array = reinterpret_cast<char *>(&ret);
//  for (int i = 0; i < sizeof(ret) / 2; ++i) {
//    std::swap(byte_array[i], byte_array[sizeof(ret) - 1 - i]);
//  }
  return ret & 0xFFFFFFFFL;
}
