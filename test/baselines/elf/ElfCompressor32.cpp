#include <cstdint>
#include <assert.h>
#include <math.h>

#include "elf.h"
#include "defs.h"
#include "BitStream/BitWriter.h"
#include "BitStream/BitReader.h"

class AbstractElfCompressor32 {
 private:
  size_t size = 32;
  int lastBetaStar = __INT32_MAX__;
 protected:
  virtual int writeInt(int n, int len) = 0;
  virtual int writeBit(bool bit) = 0;
  virtual int xorCompress(int vPrimeInt) = 0;

 public:
  void addValue(float v) {
    FLOAT data = {.f = v};
    int vPrimeInt;
    assert(!isnan(v));
    if (v == 0.0) {
      size += writeInt(2, 2);
      vPrimeInt = data.i;
      // } else if (isnan(v)) {
      //         size += writeInt(2,2);
      //         vPrimeLong = 0xfff8000000000000L & data.i;
    } else {
      int *alphaAndBetaStar = getAlphaAndBetaStar_32(v, lastBetaStar);
      int e = (data.i >> 23) & 0xff;
      int gAlpha = getFAlpha(alphaAndBetaStar[0]) + e - 127;
      int eraseBits = 23 - gAlpha;
      int mask = 0xffffffff << eraseBits;
      int delta = (~mask) & data.i;
      if (delta != 0 && eraseBits > 3) {
        if (alphaAndBetaStar[1] == lastBetaStar) {
          size += writeBit(false);
        } else {
          size += writeInt(alphaAndBetaStar[1] | 0x18, 5);
          lastBetaStar = alphaAndBetaStar[1];
        }
        vPrimeInt = mask & data.i;
      } else {
        size += writeInt(2, 2);
        vPrimeInt = data.i;
      }

      delete[] alphaAndBetaStar;
    }
    size += xorCompress(vPrimeInt);
  }

  int getSize() {
    return size;
  }
};

static const short leadingRepresentation_32[] =
    {0, 0, 0, 0, 0, 0, 1, 1,
     1, 1, 2, 2, 3, 3, 4, 4,
     5, 5, 6, 6, 7, 7, 7, 7,
     7, 7, 7, 7, 7, 7, 7, 7
    };

static const short leadingRound_32[] =
    {0, 0, 0, 0, 0, 0, 6, 6,
     6, 6, 10, 10, 12, 12, 14, 14,
     16, 16, 18, 18, 20, 20, 20, 20,
     20, 20, 20, 20, 20, 20, 20, 20
    };

class ElfXORCompressor32 {
 private:
  int storedLeadingZeros = __INT32_MAX__;
  int storedTrailingZeros = __INT32_MAX__;
  uint32_t storedVal = 0;
  bool first = true;
  size_t size = 32;
  size_t length = 0;
  BitWriter writer;
  uint32_t *output;

  int writeFirst(int value) {
    first = false;
    storedVal = value;
    length = 1;
    int trailingZeros = __builtin_ctz(value);
    write(&writer, trailingZeros, 6);
    // if (trailingZeros < 64) { optimized-out somehow, assuming __builtin_ctzl always < 64 ?
    if (value != 0) {
      writeLong(&writer, storedVal >> (trailingZeros + 1), 31 - trailingZeros);
      size += 37 - trailingZeros;
      return 37 - trailingZeros;
    } else {
      size += 6;
      return 6;
    }
  }

  int compressValue(int value) {
    int thisSize = 0;
    uint32_t _xor = storedVal ^ value;
    if (_xor == 0) {
      write(&writer, 1, 2);
      size += 2;
      thisSize += 2;
    } else {
      int leadingZeros = leadingRound_32[__builtin_clz(_xor)];
      int trailingZeros = __builtin_ctz(_xor);

      if (leadingZeros == storedLeadingZeros && trailingZeros >= storedTrailingZeros) {
        int centerBits = 32 - storedLeadingZeros - storedTrailingZeros;
        int len = 2 + centerBits;
        if (len > 32) {
          write(&writer, 0, 2);
          writeLong(&writer, _xor >> storedTrailingZeros, centerBits);
        } else {
          writeLong(&writer, _xor >> storedTrailingZeros, len);
        }

        size += len;
        thisSize += len;
      } else {
        storedLeadingZeros = leadingZeros;
        storedTrailingZeros = trailingZeros;
        int centerBits = 32 - storedLeadingZeros - storedTrailingZeros;

        if (centerBits <= 8) {
          write(&writer, (((0x2 << 3) | leadingRepresentation_32[storedLeadingZeros]) << 3) | (centerBits & 0x7), 8);
          writeLong(&writer, _xor >> (storedTrailingZeros + 1), centerBits - 1);

          size += 7 + centerBits;
          thisSize += 7 + centerBits;
        } else {
          write(&writer, (((0x3 << 3) | leadingRepresentation_32[storedLeadingZeros]) << 5) | (centerBits & 0x1f), 10);
          writeLong(&writer, _xor >> (storedTrailingZeros + 1), centerBits - 1);

          size += 9 + centerBits;
          thisSize += 9 + centerBits;
        }
      }
      storedVal = value;
    }
    length++;
    return thisSize;
  }

 public:
  BitWriter *getWriter() {
    return &writer;
  }

  void init(size_t length) {
    length *= 12;
    output = (uint32_t *) malloc(length + 4);
    initBitWriter(&writer, output + 1, length / sizeof(uint32_t));
  }

  int addValue(int value) {
    if (first) {
      return writeFirst(value);
    } else {
      return compressValue(value);
    }
  }

  int addValue(float value) {
    FLOAT data = {.f = value};
    if (first) {
      return writeFirst(data.i);
    } else {
      return compressValue(data.i);
    }
  }

  void close() {
    *output = length;
    flush(&writer);
  }

  size_t getSize() {
    return size;
  }

  uint32_t *getOut() {
    return output;
  }
};

class ElfCompressor32 : public AbstractElfCompressor32 {
 private:
  ElfXORCompressor32 xorCompressor;

 protected:
  int writeInt(int n, int len) override {
    write(xorCompressor.getWriter(), n, len);
    return len;
  }

  int writeBit(bool bit) override {
    write(xorCompressor.getWriter(), bit, 1);
    return 1;
  }

  int xorCompress(int vPrimeInt) override {
    return xorCompressor.addValue(vPrimeInt);
  }

 public:
  void init(int length) {
    xorCompressor.init(length);
  }
  uint32_t *getBytes() {
    return xorCompressor.getOut();
  }

  void close() {
    writeInt(2, 2);
    xorCompressor.close();
  }
};

ssize_t elf_encode_32(float *in, ssize_t len, uint8_t **out, float error) {
  ElfCompressor32 compressor;
  compressor.init(len);
  for (int i = 0; i < len; i++) {
    if (i == 4219) {
      asm("nop");
    }
    compressor.addValue(in[i]);
  }
  compressor.close();
  *out = (uint8_t *) compressor.getBytes();
  return (compressor.getSize() + 31) / 32 * 4;
}