#include "fpc_compressor.h"

const long long FpcCompressor::mask[8] = {
        0x0000000000000000LL,
        0x00000000000000ffLL,
        0x000000000000ffffLL,
        0x0000000000ffffffLL,
        0x000000ffffffffffLL,
        0x0000ffffffffffffLL,
        0x00ffffffffffffffLL,
        static_cast<long long>(0xffffffffffffffff)
};

long FpcCompressor::getCompressedSizeInBits() {
    return compressedSizeInBits;
}

FpcCompressor::FpcCompressor(long pred, int num) {
    intot = num;
    predsizem1 = pred;

    outbuf[0] = predsizem1;
    predsizem1 = (1L << predsizem1) - 1;

    hash = 0;
    dhash = 0;
    lastval = 0;
    pred1 = 0;
    pred2 = 0;
    fcm = (long long *)calloc(predsizem1 + 1, 8);
    dfcm = (long long *)calloc(predsizem1 + 1, 8);

    out = ((intot + 1));
    *((long long *)&outbuf[(out >> 3) << 3]) = 0;
}

FpcCompressor::~FpcCompressor() {
    free(fcm);
    free(dfcm);
}

void FpcCompressor::addValue(double v) {
    memcpy(&val, &v, sizeof(double));
    xor1 = val ^ pred1;
    fcm[hash] = val;
    hash = ((hash << 6) ^ ((unsigned long long)val >> 48)) & predsizem1;
    pred1 = fcm[hash];

    stride = val - lastval;
    xor2 = val ^ (lastval + pred2);
    lastval = val;
    dfcm[dhash] = stride;
    dhash = ((dhash << 2) ^ ((unsigned long long)stride >> 40)) & predsizem1;
    pred2 = dfcm[dhash];

    code = 0;
    if ((unsigned long long)xor1 > (unsigned long long)xor2) {
        code = 0x8;
        xor1 = xor2;
    }
    bcode = 7; // 8 bytes: bcode指异或值的字节数量
    if (0 == (xor1 >> 56))
        bcode = 6; // 7 bytes
    if (0 == (xor1 >> 48))
        bcode = 5; // 6 bytes
    if (0 == (xor1 >> 40))
        bcode = 4; // 5 bytes
    if (0 == (xor1 >> 24))
        bcode = 3; // 3 bytes
    if (0 == (xor1 >> 16))
        bcode = 2; // 2 bytes
    if (0 == (xor1 >> 8))
        bcode = 1; // 1 byte
    if (0 == xor1)
        bcode = 0; // 0 bytes

    out += bcode + (bcode >> 2); // 0 ~ 8 需要写入的数据大小
    code |= bcode;
    i++;

    // 第一个outbuf写入数据时，前面的所有数据都已经稳定，可以write进数据流中
    outStream.WriteInt(code, 4);
    compressedSizeInBits += 4;

    if (bcode >= 4) {
        outStream.WriteLong(xor1, (bcode + 1) * 8);
        compressedSizeInBits += (bcode + 1) * 8;
    } else if (bcode > 0) {
        outStream.WriteLong(xor1, bcode * 8);
        compressedSizeInBits += bcode * 8;
    }
}

std::vector<char> FpcCompressor::getBytes() {
    int byteCount = std::ceil(compressedSizeInBits / 8.0);
    std::vector<char> result;
    result.reserve(byteCount);
    auto bytes = outStream.GetBuffer(byteCount);
    for (int i = 0; i < byteCount; ++i) {
        result.push_back(static_cast<char>(bytes[i]));
    }
    return result;
}

void FpcCompressor::close() {
    if (0 != (intot & 1)) {
        out -= bcode + (bcode >> 2);
    }
    i = 0;
    outStream.Flush();
}