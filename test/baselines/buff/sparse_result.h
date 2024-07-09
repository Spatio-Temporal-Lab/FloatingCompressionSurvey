#ifndef SPARSE_RESULT_H
#define SPARSE_RESULT_H


#include <vector>
#include <cstdint>

#include "array.h"

class SparseResult {
public:
    bool flag_;
    uint8_t frequent_value_ = 0;
    Array<uint8_t> bitmap_;
    Array<bool> is_frequent_value_;
    Array<uint8_t> outliers_;
    int outliers_count_ = 0;

    explicit SparseResult(int batch_size);

    void set_frequent_value(int frequent_value);

    Array<uint8_t> &get_outliers();
};


#endif // SPARSE_RESULT_H
