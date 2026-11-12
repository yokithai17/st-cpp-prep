#pragma once

#include "ring_buffer.hpp"
#include <cassert>
#include <exception>
#include <type_traits>

template <typename T> struct is_const_ref : std::false_type {};

template <typename T> struct is_const_ref<const T &> : std::true_type {};

template <typename T>
inline constexpr bool is_const_ref_v = is_const_ref<T>::value;

template <class T> class test_class {
public:
  test_class(RingBuffer<T> &br) : br(br) {}

  void pop(const T &expected) {
    auto t = br.try_pop();
    assert(t.has_value() && t.value() == expected);
  }

  void push(const T &x) { push_many(x); }

  template <typename... Args> void push_many(Args &&...args) {
    (..., (void)([this](auto &&arg) {
       bool pushed = br.try_push(std::forward<decltype(arg)>(arg));
       assert(pushed); // (помните про замену assert на исключение в релизе)
     }(std::forward<Args>(args))));
  }

  void size(std::size_t expected) { assert(br.size() == expected); }

  void declained_push(const T &x) {
    bool pushed = br.try_push(x);
    assert(!pushed);
  }

  void front(const T &expected) {
    assert(br.front() == expected);
    const auto &brc = br;

    const T &t = brc.front();
    static_assert(is_const_ref_v<decltype(brc.front())>);
    assert(t == expected);
  }

  void empty() {
    assert(br.empty());
    assert(br.size() == 0);
  }

private:
  RingBuffer<T> &br;
};

class test_count_class {
  inline static int count = 0;

public:
  test_count_class() noexcept { ++count; }

  test_count_class(const test_count_class &) = delete;

  test_count_class(test_count_class &&) noexcept { ++count; }

  ~test_count_class() noexcept { --count; }

  static int get_count() { return count; }
};

struct no_default_construct {
  no_default_construct() = delete;
  no_default_construct(int x) : x(x) {}
  int x;
};

struct construct_counts {
  inline static int moves = 0;
  inline static int copy = 0;
  inline static int defaults = 0;
  construct_counts() { ++defaults; };
  construct_counts(construct_counts &&) noexcept { ++moves; }
  construct_counts(const construct_counts &) noexcept { ++copy; }
};

struct throwable_copy_construct {
  throwable_copy_construct(throwable_copy_construct &&rhs) noexcept
      : is_throw(rhs.is_throw), cm_counts(std::move(rhs.cm_counts)) {}

  ~throwable_copy_construct() = default;

  throwable_copy_construct(const throwable_copy_construct &rhs)
      : is_throw(rhs.is_throw), cm_counts(rhs.cm_counts) {
    if (rhs.is_throw) {
      throw std::exception();
    }
  }

  throwable_copy_construct(bool is_throw, bool fail_construction = false)
      : is_throw(is_throw) {
    if (fail_construction) {
      throw std::exception();
    }
  }

  static int get_count() {
    return test_count_class::get_count();
  }

  static int get_moves() {
    return construct_counts::moves;
  }

  static int get_copies() {
    return construct_counts::copy;
  }

  static int get_defaults() {
    return construct_counts::defaults;
  }

  bool is_throw = false;
  test_count_class lifes_counts;
  construct_counts cm_counts;
};


