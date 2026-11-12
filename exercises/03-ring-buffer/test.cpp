#include "ring_buffer.hpp"
#include "test_class.hpp"
#include <cassert>
#include <memory>
#include <string>
#include <deque>
#include <iostream>
#include <random>
#include <vector>

static_assert(!std::is_copy_constructible_v<RingBuffer<int>>);
static_assert(!std::is_copy_assignable_v<RingBuffer<int>>);
static_assert(!std::is_move_constructible_v<RingBuffer<int>>);
static_assert(!std::is_move_assignable_v<RingBuffer<int>>);
static_assert(std::is_same_v<decltype(std::declval<RingBuffer<int>&>().front()), int&>);
static_assert(std::is_same_v<decltype(std::declval<const RingBuffer<int>&>().front()), const int&>);

void test_empty_buffer_construct() {
  RingBuffer<int> buff(0);
  assert(buff.size() == 0);
  assert(buff.full());
  auto poped = buff.try_pop();
  assert(!poped.has_value());
  assert(buff.empty());
  assert(buff.capacity() == 0);
}

void test_normal_buffer_construct() {
  RingBuffer<int> buff(3);
  assert(buff.size() == 0);
  assert(!buff.full());
  auto poped = buff.try_pop();
  assert(!poped.has_value());
  assert(buff.empty());
  assert(buff.capacity() == 3);
}

void test_buffer_front() {
  {
    RingBuffer<int> buff(3);
    bool caught = false;
    try {
      buff.front();
    } catch (const std::out_of_range &e) {
      caught = true;
    }
    assert(caught);

    const auto &br = buff;
    caught = false;
    try {
      br.front();
    } catch (const std::out_of_range &e) {
      caught = true;
    }

    assert(caught);
  }

  {
    RingBuffer<int> buff(3);
    test_class<int> tc(buff);
    tc.push(1);

    tc.front(1);
  }

  {
    RingBuffer<int> buff(3);
    test_class<int> tc(buff);

    tc.push_many(1, 2, 3);
    tc.front(1);
  }

  {
    RingBuffer<int> buff(3);
    test_class<int> tc(buff);

    tc.push_many(1, 2, 3);

    tc.pop(1);

    tc.front(2);
  }

  {
    RingBuffer<int> b(3);
    test_class<int> tc(b);

    tc.push(1);

    auto &x = b.front();
    ++x;

    tc.size(1);

    tc.front(2);

    tc.pop(2);
  }
}

void test_buffer_correction() {
  {
    RingBuffer<int> b(3);
    test_class<int> tc(b);

    tc.push_many(10, 20, 30);

    tc.declained_push(40);

    tc.pop(10);

    tc.push(40);

    tc.pop(20);
    tc.pop(30);
    tc.pop(40);

    assert(b.empty());
  }
}

void test_buffer_count() {
  assert(test_count_class::get_count() == 0);
  {
    RingBuffer<test_count_class> b(3);
    assert(test_count_class::get_count() == 0);
  }

  assert(test_count_class::get_count() == 0);

  {
    RingBuffer<test_count_class> b(3);
    b.try_emplace();
    assert(test_count_class::get_count() == 1);
    {
      auto opt = b.try_pop();
      assert(test_count_class::get_count() == 1);
    }
    assert(test_count_class::get_count() == 0);
  }

  assert(test_count_class::get_count() == 0);

  {
    RingBuffer<test_count_class> b(3);
    b.try_emplace();
    b.try_emplace();
    assert(test_count_class::get_count() == 2);
    
    b.clear();
    assert(test_count_class::get_count() == 0);

    b.try_emplace();
    assert(test_count_class::get_count() == 1);
  }

    assert(test_count_class::get_count() == 0);
  {
    RingBuffer<test_count_class> b(3);
    b.try_emplace();
    b.try_emplace();
    assert(test_count_class::get_count() == 2);
  }
  assert(test_count_class::get_count() == 0);

  {
    RingBuffer<test_count_class> b(1);
    b.try_emplace();
    b.try_emplace();

    assert(test_count_class::get_count() == 1);
  }
}

void test_buffer_behaivour(){
    {
        RingBuffer<std::unique_ptr<int>> b(1);
        std::unique_ptr<int> p1 = std::make_unique<int>(1);
        int* x = p1.get();

        b.try_push(std::move(p1));
        assert(p1.get() == nullptr);
        assert(b.front().get() == x);

        std::unique_ptr<int> p2 = std::make_unique<int>(2);
        int* y = p2.get();
        b.try_push(std::move(p2));
        assert(p2.get() == y);

        auto opt = b.try_pop();
        assert(opt.value().get() == x);

        b.try_push(std::move(p2));
        assert(b.front().get() == y);
    }

    {
        RingBuffer<no_default_construct> b(4);
        b.try_emplace(1);
        assert(b.front().x == 1);
    }

    {
        RingBuffer<std::string> b(3);
        test_class<std::string> tc(b);
        std::string a("abc");
        tc.push_many(a, std::move(a), "acd");
        tc.pop("abc");
        tc.pop("abc");
        tc.pop("acd");
    }
}

void test_buffer_clear() {
  RingBuffer<int> b(3);
  test_class<int> tc(b);
  tc.push_many(1, 2, 3);
  tc.pop(1);
  tc.push(4);

  b.clear();

  tc.empty();

  assert(b.capacity() == 3);

  tc.push(6);
  tc.pop(6);
}

// По прямой просьбе студента: блок исключений и итоговые проверки.
template<class F> void expect_construction_failure(F operation) {
  bool caught = false;
  try { operation(); } catch (const std::exception&) { caught = true; }
  assert(caught);
}

void test_buffer_exceptions() {
  using T = throwable_copy_construct;
  assert(T::get_count() == 0);
  // Заполнение через конец массива; отказ должен сохранить оба элемента.
  for (bool copy_failure : {false, true}) {
    {
      RingBuffer<T> b(3);
      b.try_emplace(false);
      b.try_emplace(true);
      b.try_pop();
      b.try_emplace(false);
      T source(true);
      const auto live = T::get_count();
      const auto copies = T::get_copies();
      const auto defaults = T::get_defaults();
      T* first = &b.front();
      expect_construction_failure([&] {
        if (copy_failure) b.try_push(source);
        else b.try_emplace(false, true);
      });
      assert(T::get_count() == live);
      assert(T::get_copies() == copies + (copy_failure ? 1 : 0));
      assert(T::get_defaults() == defaults + (copy_failure ? 0 : 1));
      assert(b.size() == 2 && &b.front() == first && b.front().is_throw);
      // Успешная вставка после отказа, включая именно копирующую ветку.
      source.is_throw = false;
      bool inserted = copy_failure ? b.try_push(source) : b.try_emplace(false);
      assert(inserted && b.size() == 3);
      for (bool expected : {true, false, false}) {
        auto value = b.try_pop();
        assert(value.has_value() && value->is_throw == expected);
      }
      assert(b.empty());
      assert(T::get_count() == 1); // Только входной source.
    }
    assert(T::get_count() == 0);
  }
  // Ни обычный, ни copy/move-конструктор при полном буфере не вызывается.
  for (std::size_t capacity : {0u, 1u}) {
    {
      RingBuffer<T> b(capacity);
      if (capacity) b.try_emplace(false);
      T source(true);
      const auto live = T::get_count();
      const auto defaults = T::get_defaults();
      const auto copies = T::get_copies();
      const auto moves = T::get_moves();
      bool emplaced = b.try_emplace(false, true);
      bool copied = b.try_push(source);
      bool moved = b.try_push(std::move(source));
      assert(!emplaced && !copied && !moved);
      assert(source.is_throw && b.size() == capacity);
      assert(T::get_count() == live);
      assert(T::get_defaults() == defaults);
      assert(T::get_copies() == copies && T::get_moves() == moves);
      if (capacity) assert(!b.front().is_throw);
    }
    assert(T::get_count() == 0);
  }
}

void test_edges_and_references() {
  for (std::size_t capacity : {0u, 1u, 2u, 3u}) {
    RingBuffer<int> b(capacity);
    for (int round = 0; round < 12; ++round) {
      for (std::size_t i = 0; i < capacity; ++i) {
        int value = round * 10 + static_cast<int>(i);
        bool pushed = b.try_push(value);
        assert(pushed);
      }
      bool pushed = b.try_push(-1);
      assert(!pushed && b.full() && b.size() == capacity);
      for (std::size_t i = 0; i < capacity; ++i) {
        auto value = b.try_pop();
        assert(value == round * 10 + static_cast<int>(i));
      }
      b.clear();
      b.clear();
      assert(b.empty() && b.size() == 0 && b.capacity() == capacity);
      for (bool const_access : {false, true}) {
        bool caught = false;
        try {
          if (const_access) std::as_const(b).front();
          else b.front();
        } catch (const std::out_of_range&) { caught = true; }
        assert(caught);
      }
    }
  }
  RingBuffer<std::string> b(3);
  b.try_emplace("first");
  std::string& first = b.front();
  auto* address = &first;
  b.try_emplace("second");
  b.try_emplace("third");
  assert(&b.front() == address && first == "first");

  // Результат pop переживает уничтожение контейнера.
  auto result = [] {
    RingBuffer<std::unique_ptr<int>> owned(1);
    owned.try_emplace(std::make_unique<int>(42));
    return owned.try_pop();
  }();
  assert(result.has_value() && **result == 42);
}

void test_model() {
  constexpr unsigned seed = 0xC0FFEE;
  for (std::size_t capacity : {0u, 1u, 2u, 3u, 7u}) {
    std::mt19937 random(seed);
    RingBuffer<int> b(capacity);
    std::deque<int> model;
    std::vector<std::string> history;
    auto check = [&](bool condition) {
      if (condition) return;
      std::cerr << "seed=" << seed << " capacity=" << capacity << '\n';
      for (const auto& operation : history) std::cerr << operation << '\n';
      throw std::runtime_error("RingBuffer differs from model");
    };
    for (int step = 0; step < 3000; ++step) {
      unsigned operation = random() % 10;
      if (operation < 4) {
        int value = static_cast<int>(random() % 1000);
        history.push_back("push " + std::to_string(value));
        bool expected = model.size() < capacity;
        bool actual = b.try_push(value);
        check(actual == expected);
        if (expected) model.push_back(value);
      } else if (operation < 8) {
        history.push_back("pop");
        auto actual = b.try_pop();
        check(actual.has_value() == !model.empty());
        if (!model.empty()) { check(*actual == model.front()); model.pop_front(); }
      } else if (operation == 8) {
        history.push_back("clear");
        b.clear(); model.clear();
      } else {
        history.push_back("front");
        if (model.empty()) {
          bool caught = false;
          try { b.front(); } catch (const std::out_of_range&) { caught = true; }
          check(caught);
        } else check(b.front() == model.front());
      }
      check(b.size() == model.size() && b.capacity() == capacity);
      check(b.empty() == model.empty() && b.full() == (model.size() == capacity));
      if (!model.empty()) check(std::as_const(b).front() == model.front());
    }
    while (!model.empty()) {
      history.push_back("final pop");
      check(b.try_pop() == model.front());
      model.pop_front();
    }
    check(b.empty());
  }
}

int main() {
  test_empty_buffer_construct();
  test_normal_buffer_construct();
  test_buffer_front();
  test_buffer_correction();
  test_buffer_clear();
  test_buffer_count();
  test_buffer_behaivour();
  test_buffer_exceptions();
  test_edges_and_references();
  test_model();
  return 0;
}
