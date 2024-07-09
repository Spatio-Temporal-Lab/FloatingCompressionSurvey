#include <cstring>
#include <cmath>
#include <vector>

#include "output_bit_stream.h"

#define SIZE 32768

class FpcCompressor {
private:
    OutputBitStream outStream = OutputBitStream(6 + (SIZE / 2) + (SIZE * 8) + 2);
    static const long long mask[8];
    long i, out, intot, hash, dhash, code, bcode, ioc;
    long long val, lastval, stride, pred1, pred2, xor1, xor2;
    long long *fcm, *dfcm;
    unsigned long long inbuf[SIZE + 1];
    unsigned char outbuf[6 + (SIZE / 2) + (SIZE * 8) + 2];
    long predsizem1;

public:
    FpcCompressor(long pred, int intot);
    ~FpcCompressor();
    void addValue(double v);
    void close();
    long getCompressedSizeInBits();
    std::vector<char> getBytes();

    int compressedSizeInBits = 0;
};