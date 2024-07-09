#include "int_encoder.h"

void IntEncoder::write(int number, std::ostringstream &output_stream) {
  output_stream.write(reinterpret_cast<char *>(&number), sizeof(number));
}

int IntEncoder::read(std::istringstream &input_stream) {
  int ret;
  input_stream.read(reinterpret_cast<char *>(&ret), sizeof(ret));
//  char *byte_array = reinterpret_cast<char *>(&ret);
//  for (int i = 0; i < sizeof(ret) / 2; ++i) {
//    std::swap(byte_array[i], byte_array[sizeof(ret) - 1 - i]);
//  }
  return ret;
}
