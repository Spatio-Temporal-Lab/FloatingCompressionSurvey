#ifndef SIM_PIECE_SIM_PIECE_SEGMENT_H_
#define SIM_PIECE_SIM_PIECE_SEGMENT_H_

class SimPieceSegment {
 public:
  SimPieceSegment(long init_timestamp, double aMin, double aMax, double b): init_timestamp_(init_timestamp),
                                                                            a_min_(aMin), a_max_(aMax), a_((aMin + aMax) / 2), b_(b) {}

  SimPieceSegment(long init_timestamp, double a, double b): SimPieceSegment(init_timestamp, a, a, b) {}

  SimPieceSegment(const SimPieceSegment& other) = default;

  long getInitTimestamp() {
    return init_timestamp_;
  }

  double getAMin() {
    return a_min_;
  }

  double getAMax() {
    return a_max_;
  }

  double getA() {
    return a_;
  }

  double getB() {
    return b_;
  }

 private:
  long init_timestamp_;
  double a_min_;
  double a_max_;
  double a_;
  double b_;
};

#endif // SIM_PIECE_SIM_PIECE_SEGMENT_H_
