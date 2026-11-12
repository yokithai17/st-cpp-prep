#pragma once

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

template <class T> class RingBuffer {
  static_assert(std::is_object_v<T> && !std::is_const_v<T> &&
                !std::is_volatile_v<T> && !std::is_array_v<T>, "Не выполнен контракт для типа");

  static_assert(std::is_nothrow_move_constructible_v<T> &&
                std::is_nothrow_destructible_v<T>, "Не выполнен контракт для noexcept");

public:
  explicit RingBuffer(std::size_t capacity) : M_buffer_(capacity) {}

  RingBuffer(const RingBuffer &) = delete;
  RingBuffer &operator=(const RingBuffer &) = delete;
  RingBuffer(RingBuffer &&) = delete;
  RingBuffer &operator=(RingBuffer &&) = delete;

  bool try_push(const T &value)
    requires std::is_copy_constructible_v<T>
  {
    return try_emplace(value);
  }

  bool try_push(T &&value) { return try_emplace(std::move(value)); }

  template <class... Args>
  bool try_emplace(Args &&...args)
    requires std::is_constructible_v<T, Args...>
  {
    if (full()) {
      return false;
    }

    auto insert_index = get_index(M_begin_ + size());

    auto &opt = M_buffer_[insert_index];

    opt.emplace(std::forward<Args>(args)...);

    M_size_++;
    return true;
  }

  std::optional<T> try_pop() noexcept {
    if (empty()) {
      return std::nullopt;
    }

    auto &&pop_val = M_buffer_[M_begin_];
    std::optional<T> val(std::move(pop_val));
    pop_val.reset();

    M_begin_ = get_index(M_begin_ + 1);
    --M_size_;

    return val;
  }

  T &front() {
    if (empty()) {
      throw std::out_of_range("");
    }

    return M_buffer_[M_begin_].value();
  }

  const T &front() const {
    if (empty()) {
      throw std::out_of_range("");
    }

    return M_buffer_[M_begin_].value();
  }

  void clear() noexcept {
    for (std::size_t i = 0; i < size(); ++i) {
      std::size_t idx = get_index(i + M_begin_);
      M_buffer_[idx].reset();
    }
    M_size_ = 0;
    M_begin_ = 0;
  }

  std::size_t size() const noexcept { return M_size_; }

  std::size_t capacity() const noexcept { return M_buffer_.size(); }

  bool empty() const noexcept { return size() == 0; }

  bool full() const noexcept { return capacity() == size(); }

private:
  std::vector<std::optional<T>> M_buffer_;
  std::size_t M_begin_ = 0;
  std::size_t M_size_ = 0;

  void swap(RingBuffer<T> &other) noexcept {
    using std::swap;
    swap(M_buffer_, other.M_buffer_);
    swap(M_begin_, other.M_begin_);
    swap(M_size_, other.M_size_);
  }

  inline std::size_t get_index(std::size_t i) const noexcept {
    return i % capacity();
  }
};
