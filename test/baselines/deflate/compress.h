
#define ZLIB_INTERNAL
#include "zlib.h"

int ZEXPORT compress2(Bytef *dest, uLongf *destLen, const Bytef *source,
                      uLong sourceLen, int level);
int ZEXPORT compress(Bytef *dest, uLongf *destLen, const Bytef *source,
                     uLong sourceLen);
uLong ZEXPORT compressBound(uLong sourceLen);
