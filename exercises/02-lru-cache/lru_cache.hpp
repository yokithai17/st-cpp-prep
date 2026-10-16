#pragma once

#include <cstddef>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>

class LruCache {
public:
  explicit LruCache(std::size_t capacity) : capacity_(capacity) {}

  const std::string *get(int key) {
    auto it = map_.find(key);
    if (it == map_.end()) {
      return nullptr;
    }

    list_.splice(list_.begin(), list_, it->second);
    auto list_it = it->second;

    return &list_it->second;
  }

  void put(int key, std::string value) {
    if (capacity() == 0) {
      return;
    }

    auto it = map_.find(key);

    if (it != map_.end()) {
      auto list_it = it->second;
      list_.splice(list_.begin(), list_, it->second);
      list_it->second = std::move(value);
      return;
    }

    if (size() == capacity()) {
      erase_last_usage();
    }

    list_.emplace(list_.begin(), key, std::move(value));
    try {
      map_[key] = list_.begin();
    } catch (...) {
      list_.pop_front();
      throw;
    }
  }

  bool erase(int key) {
    auto it = map_.find(key);
    if (it == map_.end()) {
      return false;
    }

    list_.erase(it->second);
    map_.erase(it);

    return true;
  }

  std::size_t size() const noexcept { return list_.size(); }
  std::size_t capacity() const noexcept { return capacity_; }

  LruCache(const LruCache &) = delete;
  LruCache &operator=(const LruCache &) = delete;
  LruCache(LruCache &&) = delete;
  LruCache &operator=(LruCache &&) = delete;

private:
  void erase_last_usage() {
    auto it_list = std::prev(list_.end());
    auto &[key, str] = *it_list;
    erase(key);
  }

  using ListType = std::pair<int, std::string>;
  using ListIt = std::list<ListType>::iterator;

  std::list<ListType> list_;
  std::unordered_map<int, ListIt> map_;
  std::size_t capacity_;
};
