#pragma once

#include <cstddef>
#include <vector>

namespace cb {

class CircularBuffer {
public:
    explicit CircularBuffer(int capacity)
        : buf_(capacity > 0 ? static_cast<std::size_t>(capacity) : 0) {}

    void add(double value) {
        buf_[head_] = value;
        ++head_;
        if (head_ >= buf_.size()) head_ = 0;
        if (size_ < buf_.size()) ++size_;
    }

    std::vector<double> get(int amount) const {
        std::vector<double> out;

        const std::size_t n = std::min(static_cast<std::size_t>(amount), size_);
        const std::size_t start = size_ - n;
        out.reserve(n);
        for (std::size_t i = 0; i < n; ++i) {
            out.push_back(buf_[(head_ + start - n + i) % buf_.size()]);
        }
        return out;
    }

private:
    std::vector<double> buf_;
    std::size_t head_ = 0;
    std::size_t size_ = 0;
};

}