#include "lz4_compressor.h"

LZ4Compressor::LZ4Compressor(int block_size) {
  compress_frame = Array<char>(static_cast<int>(LZ4F_compressBound(block_size * sizeof(double), nullptr)));
    LZ4F_errorCode_t error_code = LZ4F_createCompressionContext(&compression_context, LZ4F_VERSION);
    if (LZ4F_isError(error_code)) {
        throw std::runtime_error(LZ4F_getErrorName(error_code));
    }

    rc = LZ4F_compressBegin(compression_context, compress_frame.begin(), compress_frame.length(), nullptr);
    if (LZ4F_isError(rc)) {
        throw std::runtime_error(LZ4F_getErrorName(rc));
    }
    written_bytes += rc;
}

LZ4Compressor::~LZ4Compressor() {
    LZ4F_errorCode_t error_code = LZ4F_freeCompressionContext(compression_context);
    if (LZ4F_isError(error_code)) {
        throw std::runtime_error(LZ4F_getErrorName(error_code));
    }
}

void LZ4Compressor::addValue(double v) {
    rc = LZ4F_compressUpdate(compression_context, compress_frame.begin() + written_bytes, compress_frame.length() - written_bytes, &v,
                             sizeof(double), nullptr);
    if (LZ4F_isError(rc)) {
        throw std::runtime_error(LZ4F_getErrorName(rc));
    }
    written_bytes += rc;
}

void LZ4Compressor::addValue32(float v) {
  rc = LZ4F_compressUpdate(compression_context, compress_frame.begin() + written_bytes, compress_frame.length() - written_bytes, &v,
                           sizeof(float), nullptr);
  if (LZ4F_isError(rc)) {
    throw std::runtime_error(LZ4F_getErrorName(rc));
  }
  written_bytes += rc;
}

void LZ4Compressor::close() {
    rc = LZ4F_compressEnd(compression_context, compress_frame.begin() + written_bytes, compress_frame.length() - written_bytes, nullptr);
    if (LZ4F_isError(rc)) {
        throw std::runtime_error(LZ4F_getErrorName(rc));
    }
    written_bytes += rc;
}

Array<char> LZ4Compressor::getBytes() {
    return compress_frame;
}

long LZ4Compressor::getCompressedSizeInBits() {
    return written_bytes * 8;
}

