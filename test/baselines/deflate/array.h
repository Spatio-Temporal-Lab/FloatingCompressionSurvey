#ifndef SERF_ARRAY_H
#define SERF_ARRAY_H

#include <algorithm>
#include <initializer_list>
#include <memory>

template<typename T>
class Array {
 public:
    Array<T> () = default;

    explicit Array<T> (int length): length_(length) {
        data_ = std::make_unique<T[]>(length_);
    }

    Array<T> (std::initializer_list<T> list): length_(list.size()) {
        data_ = std::make_unique<T[]>(length_);
        std::copy(list.begin(), list.end(), begin());
    }

    Array<T> (const Array<T> &other): length_(other.length_) {
        data_ = std::make_unique<T[]>(length_);
        std::copy(other.begin(), other.end(), begin());
    }

    Array<T> &operator = (const Array<T> &right) {
        length_ = right.length_;
        data_ = std::make_unique<T[]>(right.length_);
        std::copy(right.begin(), right.end(), begin());
        return *this;
    }

    T &operator[] (int index) const {
        return data_[index];
    }

    T *begin() const {
        return data_.get();
    }

    T *end() const {
        return data_.get() + length_;
    }

    int length() const {
        return length_;
    }

 private:
    int length_ = 0;
    std::unique_ptr<T[]> data_ = nullptr;
};

#endif  // SERF_ARRAY_H
