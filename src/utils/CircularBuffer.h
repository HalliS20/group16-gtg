#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <algorithm>
#include <cstddef>
#include <vector>

using std::size_t;
using std::vector;

template <typename T = double> class CircularBuffer {
public:
	explicit CircularBuffer(int capacity)
		: buf_(capacity > 0 ? static_cast<size_t>(capacity) : 0)
	{
	}

	void add(const T& value)
	{
		buf_[head_] = value;
		++head_;
		if (head_ >= buf_.size()) {
			head_ = 0;
		}
		if (size_ < buf_.size()) {
			++size_;
		}
	}

	vector<T> get(size_t amount) const
	{
		vector<T> out;
		if (amount == 0) {
			return out;
		}

		const size_t n = std::min(amount, size_);
		out.reserve(n);
		for (size_t i = 0; i < n; ++i) {
			out.push_back(buf_[(head_ + buf_.size() - n + i) % buf_.size()]);
		}
		return out;
	}

	T getLast() const
	{
		return buf_[(head_ + buf_.size() - 1) % buf_.size()];
	}

	size_t size() const
	{
		return size_;
	}

private:
	vector<T> buf_;
	size_t head_ = 0;
	size_t size_ = 0;
};

#endif
