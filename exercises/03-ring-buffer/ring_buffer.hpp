#pragma once

#include <cstddef>
#include <optional>
#include <utility>
#include <type_traits>
#include <vector>

template <class T> class RingBuffer {

public:
  explicit RingBuffer(std::size_t capacity) : M_buffer_(capacity) {}

  RingBuffer(const RingBuffer &) = delete;
  RingBuffer &operator=(const RingBuffer &) = delete;
  RingBuffer(RingBuffer &&) = delete;
  RingBuffer &operator=(RingBuffer &&) = delete;

  bool try_push(const T &value)
    requires std::is_copy_constructible_v<T>;

  bool try_push(T &&value);

  template <class... Args>
  bool try_emplace(Args &&...args)
    requires std::is_constructible_v<T, Args...>
  {
    if (full()) {
      return false;
    }

    auto insert_index = (M_begin_ + size()) % capacity();
    auto &opt = M_buffer_[insert_index];

    opt.emplace(std::forward<Args>(args)...);

    M_size_++;
    return true;
  }

  std::optional<T> try_pop() noexcept;

  T &front();
  const T &front() const;

  void clear() noexcept;

  std::size_t size() const noexcept { return M_size_; }

  std::size_t capacity() const noexcept { return M_buffer_.size(); }

  bool empty() const noexcept { return size() == 0; }

  bool full() const noexcept { return capacity() == size(); }

private:
  std::vector<std::optional<T>> M_buffer_;
  std::size_t M_begin_ = 0;
  std::size_t M_size_ = 0;
};