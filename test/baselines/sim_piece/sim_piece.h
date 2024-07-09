#ifndef SIM_PIECE_SIM_PIECE_H_
#define SIM_PIECE_SIM_PIECE_H_

#include <vector>
#include <map>
#include <unordered_map>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <limits>
#include <cstring>

#include "point.h"
#include "sim_piece_segment.h"
#include "float_encoder.h"
#include "variable_byte_encoder.h"
#include "u_int_encoder.h"

class SimPiece {
 public:
  SimPiece(std::vector<Point> points, double epsilon);
  SimPiece(char *input, int len, bool variableByte);
  std::vector<Point> decompress();
  int toByteArray(char *dst, bool variableByte, int *timestamp_store_size);

 private:
  std::vector<SimPieceSegment> segments_;
  double epsilon_;
  long last_timestamp_;

  double quantization(double value);
  int createSegment(int start_idx, std::vector<Point> points, std::vector<SimPieceSegment> &segments);
  std::vector<SimPieceSegment> compress(std::vector<Point> points);
  std::vector<SimPieceSegment> mergePerB(std::vector<SimPieceSegment> segments);
  int toByteArrayPerBSegments(std::vector<SimPieceSegment> segments, bool variableByte,
                               std::ostringstream &out_stream);
  std::vector<SimPieceSegment> readMergedPerBSegments(bool variableByte, std::istringstream &in_stream);
  void readByteArray(char *input, int len, bool variableByte);
};

#endif // SIM_PIECE_SIM_PIECE_H_
