#ifndef SIM_PIECE_U_INT_ENCODER_H_
#define SIM_PIECE_U_INT_ENCODER_H_

#include <sstream>

class UIntEncoder {
 public:
  static void write(long number, std::ostringstream &output_stream);
  static long read(std::istringstream &input_stream);
};

#endif // SIM_PIECE_U_INT_ENCODER_H_
