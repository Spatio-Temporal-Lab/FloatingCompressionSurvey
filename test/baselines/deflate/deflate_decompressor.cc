#include "deflate_decompressor.h"

DeflateDecompressor::DeflateDecompressor() {
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK) {
        throw std::runtime_error("[Inflate Error]: Failed to init.");
    }
}

DeflateDecompressor::~DeflateDecompressor() {
    inflateEnd(&strm);
}

std::vector<double> DeflateDecompressor::decompress(const Array<unsigned char> &bs) {
    strm.avail_in = bs.length();
    strm.next_in = bs.begin();
    std::vector<double> result;
    double decompress;
    while (true) {
        strm.avail_out = sizeof(double);
        strm.next_out = reinterpret_cast<unsigned char *>(&decompress);
        ret = inflate(&strm, Z_SYNC_FLUSH);
        if (ret == Z_STREAM_ERROR) {
            throw std::runtime_error("[Inflate Error]");
        }
        result.emplace_back(decompress);
        if (ret == Z_STREAM_END) break;
    }
    return result;
}

std::vector<float> DeflateDecompressor::decompress32(const Array<unsigned char> &bs) {
  strm.avail_in = bs.length();
  strm.next_in = bs.begin();
  std::vector<float> result;
  float decompress;
  while (true) {
    strm.avail_out = sizeof(float);
    strm.next_out = reinterpret_cast<unsigned char *>(&decompress);
    ret = inflate(&strm, Z_SYNC_FLUSH);
    if (ret == Z_STREAM_ERROR) {
      throw std::runtime_error("[Inflate Error]");
    }
    result.emplace_back(decompress);
    if (ret == Z_STREAM_END) break;
  }
  return result;
}