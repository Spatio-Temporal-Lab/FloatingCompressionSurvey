#ifndef SIM_PIECE_POINT_H_
#define SIM_PIECE_POINT_H_

class Point {
 public:
  Point(long timestamp, double value): kTimestamp(timestamp), kValue(value) {}

  long getTimestamp() const {
    return kTimestamp;
  }

  double getValue() const {
    return kValue;
  }

 private:
  const long kTimestamp;
  const double kValue;
};

#endif // SIM_PIECE_POINT_H_
