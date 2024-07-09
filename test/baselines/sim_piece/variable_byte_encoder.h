#ifndef SIM_PIECE_VARIABLE_BYTE_ENCODER_H_
#define SIM_PIECE_VARIABLE_BYTE_ENCODER_H_

#include <sstream>

class VariableByteEncoder {
 public:
  static int write(int number, std::ostringstream &output_stream);
  static int read(std::istringstream &input_stream);

 private:
  static char extract7bits(int i, long val);
  static char extract7bitsmaskless(int i, long val);
};

#endif // SIM_PIECE_VARIABLE_BYTE_ENCODER_H_
