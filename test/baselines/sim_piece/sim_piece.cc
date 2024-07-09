#include "sim_piece.h"

SimPiece::SimPiece(std::vector<Point> points, double epsilon) {
  epsilon_ = epsilon;
  last_timestamp_ = points[points.size() - 1].getTimestamp();
  segments_ = mergePerB(compress(points));
}

SimPiece::SimPiece(char *input, int len, bool variableByte) {
  readByteArray(input, len, variableByte);
}

std::vector<Point> SimPiece::decompress() {
  std::vector<Point> points;

  std::sort(segments_.begin(), segments_.end(), [](SimPieceSegment& s1, SimPieceSegment& s2) {
    return s1.getInitTimestamp() < s2.getInitTimestamp();
  });

  long currentTimeStamp = segments_[0].getInitTimestamp();

  for (size_t i = 0; i < segments_.size() - 1; ++i) {
    while (currentTimeStamp < segments_[i + 1].getInitTimestamp()) {
      points.emplace_back(currentTimeStamp,
                          segments_[i].getA() * (currentTimeStamp - segments_[i].getInitTimestamp()) + segments_[i]
                          .getB());
      currentTimeStamp++;
    }
  }

  while (currentTimeStamp <= last_timestamp_) {
    points.emplace_back(currentTimeStamp,
                        segments_.back().getA() * (currentTimeStamp - segments_.back().getInitTimestamp()) + segments_
                        .back().getB());
    currentTimeStamp++;
  }

  return points;
}

int SimPiece::toByteArray(char *dst, bool variableByte, int *timestamp_store_size) {
  std::ostringstream out_stream;
  FloatEncoder::write(static_cast<float>(epsilon_), out_stream);
  *timestamp_store_size = toByteArrayPerBSegments(segments_, variableByte, out_stream);
  if (variableByte) VariableByteEncoder::write(static_cast<int>(last_timestamp_), out_stream);
  else UIntEncoder::write(last_timestamp_, out_stream);
  std::string bytes = out_stream.str();
  std::memcpy(dst, bytes.data(), bytes.size());
  return bytes.length();
}

double SimPiece::quantization(double value) {
  return std::round(value / epsilon_) * epsilon_;
}

int SimPiece::createSegment(int start_idx, std::vector<Point> points, std::vector<SimPieceSegment> &segments) {
  long initTimestamp = points[start_idx].getTimestamp();
  double b = quantization(points[start_idx].getValue());

  if (start_idx + 1 == points.size()) {
    segments.emplace_back(initTimestamp, -std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), b);
    return start_idx + 1;
  }

  double aMax = ((points[start_idx + 1].getValue() + epsilon_) - b) / (points[start_idx + 1].getTimestamp() -
      initTimestamp);
  double aMin = ((points[start_idx + 1].getValue() - epsilon_) - b) / (points[start_idx + 1].getTimestamp() -
      initTimestamp);

  if (start_idx + 2 == points.size()) {
    segments.emplace_back(initTimestamp, aMin, aMax, b);
    return start_idx + 2;
  }

  for (size_t idx = start_idx + 2; idx < points.size(); ++idx) {
    double upValue = points[idx].getValue() + epsilon_;
    double downValue = points[idx].getValue() - epsilon_;

    double upLim = aMax * (points[idx].getTimestamp() - initTimestamp) + b;
    double downLim = aMin * (points[idx].getTimestamp() - initTimestamp) + b;

    if (downValue > upLim || upValue < downLim) {
      segments.emplace_back(initTimestamp, aMin, aMax, b);
      return idx;
    }

    if (upValue < upLim)
      aMax = std::max((upValue - b) / (points[idx].getTimestamp() - initTimestamp), aMin);
    if (downValue > downLim)
      aMin = std::min((downValue - b) / (points[idx].getTimestamp() - initTimestamp), aMax);
  }

  segments.emplace_back(initTimestamp, aMin, aMax, b);

  return points.size();
}

std::vector<SimPieceSegment> SimPiece::compress(std::vector<Point> points) {
  std::vector<SimPieceSegment> segments;
  int currentIdx = 0;
  while (currentIdx < points.size())
    currentIdx = createSegment(currentIdx, points, segments);

  return segments;
}

std::vector<SimPieceSegment> SimPiece::mergePerB(std::vector<SimPieceSegment> segments) {
  double aMinTemp = -std::numeric_limits<double>::max();
  double aMaxTemp = std::numeric_limits<double>::max();
  double b = std::numeric_limits<double>::quiet_NaN();
  std::vector<long> timestamps;
  std::vector<SimPieceSegment> mergedSegments;

  std::sort(segments.begin(), segments.end(), [](SimPieceSegment& s1, SimPieceSegment& s2) {
    if (s1.getB() == s2.getB())
      return s1.getA() < s2.getA();
    return s1.getB() < s2.getB();
  });

  for (size_t i = 0; i < segments.size(); ++i) {
    if (b != segments[i].getB()) {
      if (timestamps.size() == 1) {
        mergedSegments.emplace_back(timestamps[0], aMinTemp, aMaxTemp, b);
      } else {
        for (long timestamp : timestamps) {
          mergedSegments.emplace_back(timestamp, aMinTemp, aMaxTemp, b);
        }
      }
      timestamps.clear();
      timestamps.emplace_back(segments[i].getInitTimestamp());
      aMinTemp = segments[i].getAMin();
      aMaxTemp = segments[i].getAMax();
      b = segments[i].getB();
      continue;
    }
    if (segments[i].getAMin() <= aMaxTemp && segments[i].getAMax() >= aMinTemp) {
      timestamps.emplace_back(segments[i].getInitTimestamp());
      aMinTemp = std::max(aMinTemp, segments[i].getAMin());
      aMaxTemp = std::min(aMaxTemp, segments[i].getAMax());
    } else {
      if (timestamps.size() == 1) {
        mergedSegments.emplace_back(segments[i - 1]);
      } else {
        for (long timestamp : timestamps) {
          mergedSegments.emplace_back(timestamp, aMinTemp, aMaxTemp, b);
        }
      }
      timestamps.clear();
      timestamps.push_back(segments[i].getInitTimestamp());
      aMinTemp = segments[i].getAMin();
      aMaxTemp = segments[i].getAMax();
    }
  }
  if (!timestamps.empty()) {
    if (timestamps.size() == 1) {
      mergedSegments.emplace_back(timestamps[0], aMinTemp, aMaxTemp, b);
    } else {
      for (long timestamp : timestamps) {
        mergedSegments.emplace_back(timestamp, aMinTemp, aMaxTemp, b);
      }
    }
  }

  return mergedSegments;
}

int SimPiece::toByteArrayPerBSegments(std::vector<SimPieceSegment> segments, bool variableByte,
                                       std::ostringstream &out_stream) {
  std::map<int, std::unordered_map<double, std::vector<long>>> input;
  for (auto &segment : segments) {
    double a = segment.getA();
    int b = static_cast<int>(std::round(segment.getB() / epsilon_));
    long t = segment.getInitTimestamp();
    if (input.find(b) == input.end())
      input.insert(std::make_pair(b, std::unordered_map<double, std::vector<long>>()));
    if (input.find(b)->second.find(a) == input.find(b)->second.end())
      input.find(b)->second.insert(std::make_pair(a, std::vector<long>()));
    input.find(b)->second.find(a)->second.emplace_back(t);
  }

  int timestamp_store_bytes = 0;

  VariableByteEncoder::write(input.size(), out_stream);
  if (input.empty()) return -1;
  int previousB = input.begin()->first;
  VariableByteEncoder::write(previousB, out_stream);
  for (auto &bSegments : input) {
    VariableByteEncoder::write(bSegments.first - previousB, out_stream);
    previousB = bSegments.first;
    VariableByteEncoder::write(bSegments.second.size(), out_stream);
    for (auto &aSegment : bSegments.second) {
      FloatEncoder::write(aSegment.first, out_stream);
      if (variableByte) std::sort(aSegment.second.begin(), aSegment.second.end());
      timestamp_store_bytes += VariableByteEncoder::write(aSegment.second.size(), out_stream);
      long previousTS = 0;
      for (const auto &timestamp : aSegment.second) {
        if (variableByte) timestamp_store_bytes += VariableByteEncoder::write(timestamp - previousTS, out_stream);
        else UIntEncoder::write(timestamp, out_stream);
        previousTS = timestamp;
      }
    }
  }

  return timestamp_store_bytes;
}

std::vector<SimPieceSegment> SimPiece::readMergedPerBSegments(bool variableByte, std::istringstream &in_stream) {
  std::vector<SimPieceSegment> segments;
  long numB = VariableByteEncoder::read(in_stream);
  if (numB == 0) return segments;
  int previousB = VariableByteEncoder::read(in_stream);
  for (int i = 0; i < numB; ++i) {
    int b = VariableByteEncoder::read(in_stream) + previousB;
    previousB = b;
    int numA = VariableByteEncoder::read(in_stream);
    for (int j = 0; j < numA; ++j) {
      float a = FloatEncoder::read(in_stream);
      int numTimestamp = VariableByteEncoder::read(in_stream);
      long timestamp = 0;
      for (int k = 0; k < numTimestamp; ++k) {
        if (variableByte) timestamp += VariableByteEncoder::read(in_stream);
        else timestamp = UIntEncoder::read(in_stream);
        segments.emplace_back(timestamp, a, b * epsilon_);
      }
    }
  }

  return segments;
}

void SimPiece::readByteArray(char *input, int len, bool variableByte) {
  std::istringstream in_stream(std::string(input, len));
  this->epsilon_ = FloatEncoder::read(in_stream);
  this->segments_ = readMergedPerBSegments(variableByte, in_stream);
  if (variableByte) this->last_timestamp_ = VariableByteEncoder::read(in_stream);
  else this->last_timestamp_ = UIntEncoder::read(in_stream);
}
