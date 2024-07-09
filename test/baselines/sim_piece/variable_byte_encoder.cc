#include "variable_byte_encoder.h"

int VariableByteEncoder::write(int number, std::ostringstream &output_stream) {
  const long val = number & 0xFFFFFFFFL;

  int written_bytes;
  char buffer;
  if (val < (1 << 7)) {
    buffer = static_cast<char>((val | (1 << 7)));
    output_stream.write(&buffer, sizeof(char));
    written_bytes = 1;
  } else if (val < (1 << 14)) {
    buffer = extract7bits(0, val);
    output_stream.write(&buffer, sizeof(char));
    buffer = static_cast<char>((extract7bitsmaskless(1, val) | (1 << 7)));
    output_stream.write(&buffer, sizeof(char));
    written_bytes = 2;
  } else if (val < (1 << 21)) {
    buffer = extract7bits(0, val);
    output_stream.write(&buffer, sizeof(char));
    buffer = extract7bits(1, val);
    output_stream.write(&buffer, sizeof(char));
    buffer = static_cast<char>(extract7bitsmaskless(2, val) | (1 << 7));
    output_stream.write(&buffer, sizeof(char));
    written_bytes = 3;
  } else if (val < (1 << 28)) {
    buffer = extract7bits(0, val);
    output_stream.write(&buffer, sizeof(char));
    buffer = extract7bits(1, val);
    output_stream.write(&buffer, sizeof(char));
    buffer = extract7bits(2, val);
    output_stream.write(&buffer, sizeof(char));
    buffer = static_cast<char>(extract7bitsmaskless(3, val) | (1 << 7));
    output_stream.write(&buffer, sizeof(char));
    written_bytes = 4;
  } else {
    buffer = extract7bits(0, val);
    output_stream.write(&buffer, sizeof(char));
    buffer = extract7bits(1, val);
    output_stream.write(&buffer, sizeof(char));
    buffer = extract7bits(2, val);
    output_stream.write(&buffer, sizeof(char));
    buffer = extract7bits(3, val);
    output_stream.write(&buffer, sizeof(char));
    buffer = static_cast<char>(extract7bitsmaskless(4, val) | (1 << 7));
    output_stream.write(&buffer, sizeof(char));
    written_bytes = 5;
  }

  return written_bytes;
}

int VariableByteEncoder::read(std::istringstream &input_stream) {
  char in;
  int number;

  input_stream.read(&in, sizeof(char));
  number = in & 0x7F;
  if (in < 0) return number;

  input_stream.read(&in, sizeof(char));
  number = ((in & 0x7F) << 7) | number;
  if (in < 0) return number;

  input_stream.read(&in, sizeof(char));
  number = ((in & 0x7F) << 14) | number;
  if (in < 0) return number;

  input_stream.read(&in, sizeof(char));
  number = ((in & 0x7F) << 21) | number;
  if (in < 0) return number;

  input_stream.read(&in, sizeof(char));
  number = ((in & 0x7F) << 28) | number;

  return number;
}

char VariableByteEncoder::extract7bits(int i, long val) {
  return static_cast<char>(((val >> (7 * i)) & ((1 << 7) - 1)));
}

char VariableByteEncoder::extract7bitsmaskless(int i, long val) {
  return static_cast<char>(((val >> (7 * i))));
}
