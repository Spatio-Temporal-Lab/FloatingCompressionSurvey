#ifndef SIM_PIECE_FLOAT_ENCODER_H_
#define SIM_PIECE_FLOAT_ENCODER_H_

#include <sstream>

#include "float.h"
#include "int_encoder.h"

class FloatEncoder {
 public:
  static void write(float number, std::ostringstream &output_stream);
  static float read(std::istringstream &input_stream);
};

#endif // SIM_PIECE_FLOAT_ENCODER_H_
