#include "sparse_result.h"

SparseResult::SparseResult(int batch_size) {
    flag_ = false;
    bitmap_ = Array<uint8_t>(batch_size / 8 + 1);
    outliers_ = Array<uint8_t>(batch_size);
    is_frequent_value_ = Array<bool>(batch_size);
}

void SparseResult::set_frequent_value(int frequent_value) {
    frequent_value_ = (uint8_t) frequent_value;
}

Array<uint8_t> &SparseResult::get_outliers() {
    return outliers_;
}
