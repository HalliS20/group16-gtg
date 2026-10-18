#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

namespace cb {

template <typename T = double>
class CircularBuffer {
public:
	explicit CircularBuffer(int capacity) : buf_(capacity > 0 ? static_cast<std::size_t>(capacity) : 0){}

	void add(const T& value) {
		buf_[head_] = value;
		++head_;
		if (head_ >= buf_.size())
			head_ = 0;
		if (size_ < buf_.size())
			++size_;
	}

	std::vector<T> get(int amount) const {
		std::vector<T> out;
		if (amount <= 0)
			return out;

		const std::size_t n = std::min(static_cast<std::size_t>(amount), size_);
		out.reserve(n);
		for (std::size_t i = 0; i < n; ++i) {
			out.push_back(buf_[(head_ + buf_.size() - n + i) % buf_.size()]);
		}
		return out;
	}

	size_t size() {
		return size_;
	}

private:
	std::vector<T> buf_;
	std::size_t head_ = 0;
	std::size_t size_ = 0;
};

}