#include <cstdint>
#include <assert.h>
#include <math.h>

#include "elf.h"
#include "defs.h"
#include "BitStream/BitWriter.h"
#include "BitStream/BitReader.h"

class AbstractElfDecompressor32 {
 private:
  int lastBetaStar = __INT32_MAX__;

  float nextValue() {
    float v;
    if (readInt(1) == 0) {
      v = recoverVByBetaStar();
    } else if (readInt(1) == 0) {
      v = xorDecompress();
    } else {
      lastBetaStar = readInt(3);
      v = recoverVByBetaStar();
    }
    return v;
  }

  float recoverVByBetaStar() {
    float v;
    float vPrime = xorDecompress();
    int sp = getSP(abs(vPrime));
    if (lastBetaStar == 0) {
      v = get10iN_32(-sp - 1);
      if (vPrime < 0) {
        v = -v;
      }
    } else {
      int alpha = lastBetaStar - sp - 1;
      v = roundUp_32(vPrime, alpha);
    }
    return v;
  }
 protected:
  virtual float xorDecompress() = 0;
  virtual int readInt(int len) = 0;
  virtual int getLength() = 0;
 public:
  int decompress(float *output) {
    int len = getLength();
    for (int i = 0; i < len; i++) {
      if (i == 4219) {
        asm("nop");
      }
      output[i] = nextValue();
    }
    return len;
  }
};

static const short leadingRepresentation_32[] =
    {0, 6, 10, 12, 14, 16, 18, 20};

class ElfXORDecompressor32 {
 private:
  FLOAT storedVal = {.i = 0};
  int storedLeadingZeros = __INT32_MAX__;
  int storedTrailingZeros = __INT32_MAX__;
  bool first = true;

  BitReader reader;

  void next() {
    if (first) {
      first = false;
      int trailingZeros = peek(&reader, 6);
      forward(&reader, 6);
      if (trailingZeros < 32) {
        storedVal.i = ((readInt(&reader, 31 - trailingZeros) << 1) + 1) << trailingZeros;
      } else {
        storedVal.i = 0;
      }
    } else {
      nextValue();
    }
  }

  void nextValue() {
    int value;
    int centerBits;
    uint32_t leadAndCenter;
    int flag = peek(&reader, 2);
    forward(&reader, 2);
    switch (flag) {
      case 3:leadAndCenter = peek(&reader, 8);
        forward(&reader, 8);
        storedLeadingZeros = leadingRepresentation_32[leadAndCenter >> 5];
        centerBits = leadAndCenter & 0x1f;
        if (centerBits == 0) {
          centerBits = 32;
        }
        storedTrailingZeros = 32 - storedLeadingZeros - centerBits;
        value = ((readInt(&reader, centerBits - 1) << 1) + 1) << storedTrailingZeros;
        value = storedVal.i ^ value;
        storedVal.i = value;
        break;
      case 2:leadAndCenter = peek(&reader, 6);
        forward(&reader, 6);
        storedLeadingZeros = leadingRepresentation_32[leadAndCenter >> 3];
        centerBits = leadAndCenter & 0x7;
        if (centerBits == 0) {
          centerBits = 8;
        }
        storedTrailingZeros = 32 - storedLeadingZeros - centerBits;
        value = ((readInt(&reader, centerBits - 1) << 1) + 1) << storedTrailingZeros;
        value = storedVal.i ^ value;
        storedVal.i = value;
        break;
      case 1:break;
      default:centerBits = 32 - storedLeadingZeros - storedTrailingZeros;
        value = readInt(&reader, centerBits) << storedTrailingZeros;
        value = storedVal.i ^ value;
        storedVal.i = value;
        break;
    }
  }
 public:
  size_t length = 0;

  void init(uint32_t *in, size_t len) {
    initBitReader(&reader, in + 1, len - 1);
    length = in[0];
  }

  float *getValues() {
    float *res = (float *) malloc(sizeof(float ) * length);
    for (int i = 0; i < length; i++) {
      res[i] = readValue();
    }
    return res;
  }

  BitReader *getReader() {
    return &reader;
  }

  float readValue() {
    next();
    return storedVal.f;
  }
};

class ElfDecompressor32 : public AbstractElfDecompressor32 {
 private:
  ElfXORDecompressor32 xorDecompressor;
 protected:

  float xorDecompress() override {
    return xorDecompressor.readValue();
  }

  int readInt(int len) override {
    int res = peek(xorDecompressor.getReader(), len);
    forward(xorDecompressor.getReader(), len);
    return res;
  }

  int getLength() override {
    return xorDecompressor.length;
  }
 public:
  ElfDecompressor32(uint8_t *in, size_t len) {
    xorDecompressor.init((uint32_t *) in, len / 4);
  }
};

ssize_t elf_decode_32(uint8_t *in, ssize_t len, float *out, float error) {
  ElfDecompressor32 decompressor(in, len);
  return decompressor.decompress(out);
}