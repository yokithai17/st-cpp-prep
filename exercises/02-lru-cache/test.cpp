#include "lru_cache.hpp"

#include <iostream>

int test_get() {
  bool test_get = false;
  const std::string *p = nullptr;
  {
    LruCache t(3);
    t.put(1, "A");
    p = t.get(1);
    test_get = p != nullptr && *p == "A";

    t.put(2, "B");
    p = t.get(2);
    test_get &= p != nullptr && *p == "B";

    t.put(3, "C");
    p = t.get(3);
    test_get &= p != nullptr && *p == "C";
  }

  if (!test_get) {
    std::cerr << "Ошибка в test_get()\n";
    return 1;
  }

  bool update_value = false;
  {
    LruCache t(3);

    t.put(1, "A");
    const auto* p = t.get(1);
    
    t.put(1, "B");
    update_value = p != nullptr && *p == "B";
  }

  if (!update_value) {
    std::cerr << "Ошибка в test_get() в update value under point\n";
    return 1;
  }

  return 0;
}

int test_put() {
  bool put_less_cap = false;
  {
    LruCache t(2);

    t.put(1, "A");
    t.put(2, "B");

    const std::string *p1 = t.get(1);
    const std::string *p2 = t.get(2);

    put_less_cap =
        (p1 != nullptr && *p1 == "A") && (p2 != nullptr && *p2 == "B");
  }

  if (!put_less_cap) {
    std::cerr << "Ошибка в test_put() в базовом добавление\n";
    return 1;
  }

  bool cross_put = false;
  {
    LruCache t(2);

    t.put(1, "A");
    t.put(1, "B");

    const std::string *p = t.get(1);

    cross_put = p != nullptr && *p == "B";
  }

  if (!cross_put) {
    std::cerr << "Ошибка в test_put() в кросс добавке\n";
    return 1;
  }

  bool capacity_put = false;
  {
    LruCache t(2);

    t.put(1, "A");
    t.put(2, "B");

    t.put(1, "C");

    const std::string *p = t.get(1);
    capacity_put = p != nullptr && *p == "C";

    t.put(3, "D");

    p = t.get(3);

    capacity_put &= (p != nullptr) && (*p == "D") && !t.get(2);
  }

  if (!capacity_put) {
    std::cerr << "Ошибка в test_put() в добавлению когда size == capacity\n";
    return 1;
  }

  return 0;
}

int test_erase() {
  bool test_exist = false;
  {
    LruCache t(2);
    t.put(1, "G");

    test_exist = t.erase(1) && (t.get(1) == nullptr);
  }

  if (!test_exist) {
    std::cerr << "Ошибка в test_erase() при удаление существующенго \n";
    return 1;
  }

  bool test_uknown = false;
  {
    LruCache t(2);
    t.put(1, "G");
    const std::string *p = t.get(1);
    test_uknown = !t.erase(2) && (p != nullptr && *p == "G");
  }

  if (!test_uknown) {
    std::cerr << "Ошибка в test_erase() при удаление не существующего\n";
    return 1;
  }

  return 0;
}

int test_size() {
  LruCache t(3);

  bool is_right_size = false;

  t.put(1, "A");
  t.put(2, "B");

  is_right_size = t.size() == 2;

  t.put(1, "C");

  is_right_size &= t.size() == 2;

  if (!is_right_size) {
    std::cerr << "Ошибка в test_size()\n";
    return 1;
  }

  return 0;
}

int test_capacity() {
  LruCache t(1234);

  if (t.capacity() != 1234) {
    std::cerr << "Ошибка в test_capacity()\n";
    return 1;
  }

  bool zero_capacity = false;
  {
    LruCache x(0);
    x.put(1, "DSA");

    zero_capacity = x.get(1) == nullptr && !x.erase(1) && x.size() == 0;
  }

  if (!zero_capacity) {
    std::cerr << "Ошибка в test_capacity() zero cap\n";
    return 1;
  }

  return 0;
}

int main() {
  return test_size() | test_capacity() | test_put() | test_erase() | test_get();
}
