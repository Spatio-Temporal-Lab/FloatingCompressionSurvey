#include "lz4_decompressor.h"

LZ4Decompressor::LZ4Decompressor() {
    LZ4F_errorCode_t error_code = LZ4F_createDecompressionContext(&decompression_context, LZ4F_VERSION);
    if (LZ4F_isError(error_code)) {
        throw std::runtime_error(LZ4F_getErrorName(error_code));
    }
}

LZ4Decompressor::~LZ4Decompressor() {
    LZ4F_errorCode_t error_code = LZ4F_freeDecompressionContext(decompression_context);
    if (LZ4F_isError(error_code)) {
        throw std::runtime_error(LZ4F_getErrorName(error_code));
    }
}

std::vector<double> LZ4Decompressor::decompress(const Array<char> &bs) {
    size_t rc;
    std::vector<double> result;
    double decompressed_double;
    size_t decompress_write_bytes = sizeof(double);
    size_t decompress_read_bytes = bs.length();
    size_t decompress_read_bytes_total = 0;
    while (true) {
        rc = LZ4F_decompress(decompression_context, &decompressed_double, &decompress_write_bytes, bs.begin() + decompress_read_bytes_total, &decompress_read_bytes,
                             nullptr);
        if (LZ4F_isError(rc)) {
            throw std::runtime_error(LZ4F_getErrorName(rc));
        }
        result.emplace_back(decompressed_double);
        decompress_read_bytes_total += decompress_read_bytes;
        decompress_read_bytes = bs.length() - decompress_read_bytes_total;
        if (rc == 0) break;
    }
    return result;
}

std::vector<float> LZ4Decompressor::decompress32(const Array<char> &bs) {
  size_t rc;
  std::vector<float> result;
  float decompressed_double;
  size_t decompress_write_bytes = sizeof(float);
  size_t decompress_read_bytes = bs.length();
  size_t decompress_read_bytes_total = 0;
  while (true) {
    rc = LZ4F_decompress(decompression_context, &decompressed_double, &decompress_write_bytes, bs.begin() + decompress_read_bytes_total, &decompress_read_bytes,
                         nullptr);
    if (LZ4F_isError(rc)) {
      throw std::runtime_error(LZ4F_getErrorName(rc));
    }
    result.emplace_back(decompressed_double);
    decompress_read_bytes_total += decompress_read_bytes;
    decompress_read_bytes = bs.length() - decompress_read_bytes_total;
    if (rc == 0) break;
  }
  return result;
}
