#include "deflate_compressor.h"

DeflateCompressor::DeflateCompressor(int block_size) {
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
  compress_pack = Array<unsigned char>(2 * block_size * static_cast<int>(sizeof(double)));
  deflateInit(&strm, Z_DEFAULT_COMPRESSION);
  strm.avail_out = 2 * block_size * static_cast<int>(sizeof(double));
    strm.next_out = compress_pack.begin();
}

DeflateCompressor::~DeflateCompressor() {
    deflateEnd(&strm);
}

void DeflateCompressor::addValue(double v) {
    strm.avail_in = sizeof(double);
    strm.next_in = reinterpret_cast<unsigned char *>(&v);
  deflate(&strm, Z_NO_FLUSH);
}

void DeflateCompressor::addValue32(float v) {
  strm.avail_in = sizeof(float);
  strm.next_in = reinterpret_cast<unsigned char *>(&v);
  deflate(&strm, Z_NO_FLUSH);
}

void DeflateCompressor::close() {
    ret = deflate(&strm, Z_FINISH);
}

Array<unsigned char> DeflateCompressor::getBytes() {
    return compress_pack;
}

long DeflateCompressor::getCompressedSizeInBits() {
  return (compress_pack.length() - strm.avail_out) * 8;
}
