#include "fpc_decompressor.h"

const long long FpcDecompressor::mask[8] = {
        0x0000000000000000LL,
        0x00000000000000ffLL,
        0x000000000000ffffLL,
        0x0000000000ffffffLL,
        0x000000ffffffffffLL,
        0x0000ffffffffffffLL,
        0x00ffffffffffffffLL,
        static_cast<long long>(0xffffffffffffffff)
};

FpcDecompressor::FpcDecompressor(long pred, int num) {
    predsizem1 = pred;
    intot = num;
    predsizem1 = (1L << predsizem1) - 1;

    hash = 0;
    dhash = 0;
    lastval = 0;
    pred1 = 0;
    pred2 = 0;
    fcm = (long long *)calloc(predsizem1 + 1, 8);
    assert(NULL != fcm);
    dfcm = (long long *)calloc(predsizem1 + 1, 8);
    assert(NULL != dfcm);
}

FpcDecompressor::~FpcDecompressor() {
    free(fcm);
    free(dfcm);
}

void FpcDecompressor::setBytes(char *data, size_t data_size) {
    // Only for tmp use. With risk of memory leak
    // This awful structure must be refactored
    inStream = *(new InputBitStream(reinterpret_cast<uint8_t *>(data), data_size));
}

std::vector<double> FpcDecompressor::decompress() {
    in = ((intot + 1));
    for (i = 0; i < intot; i++) {
        code = inStream.ReadInt(4);
        bcode = code & 0x7;

        if (bcode >= 4)
            val = inStream.ReadLong(8 * (bcode + 1));
        else if (bcode > 0)
            val = inStream.ReadLong(8 * bcode);
        else
            val = 0;

        val &= mask[bcode];
        in += bcode + (bcode >> 2);

        if (0 != (code & 0x8))
            pred1 = pred2;
        val ^= pred1;

        fcm[hash] = val;
        hash = ((hash << 6) ^ ((unsigned long long)val >> 48)) & predsizem1;
        pred1 = fcm[hash];

        stride = val - lastval;
        dfcm[dhash] = stride;
        dhash = ((dhash << 2) ^ ((unsigned long long)stride >> 40)) & predsizem1;
        pred2 = val + dfcm[dhash];
        lastval = val;

        outbuf[i] = val;
    }
    std::transform(outbuf, outbuf + intot, std::back_inserter(result), [](long long opr)
                   { return *reinterpret_cast<double *>(&opr); });
    return result;
}
