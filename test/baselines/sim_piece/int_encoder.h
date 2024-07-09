#ifndef SIM_PIECE_INT_ENCODER_H_
#define SIM_PIECE_INT_ENCODER_H_

#include <sstream>

class IntEncoder {
 public:
  static void write(int number, std::ostringstream &output_stream);
  static int read(std::istringstream &input_stream);
};

#endif // SIM_PIECE_INT_ENCODER_H_
