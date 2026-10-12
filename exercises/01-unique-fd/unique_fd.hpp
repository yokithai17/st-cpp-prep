#pragma once 

#include <unistd.h>

class UniqueFd {
public:
  explicit UniqueFd(int fd = -1) noexcept : fd_(fd) {}

  ~UniqueFd() { reset(); }

  UniqueFd(const UniqueFd &) = delete;
  UniqueFd& operator=(const UniqueFd &) = delete;

  UniqueFd(UniqueFd &&rhs) noexcept : fd_(rhs.release()) {}

  UniqueFd& operator=(UniqueFd && rhs) noexcept {
    if (this != &rhs) {
      reset(rhs.release());
    }

    return *this;
  }

  int release() noexcept {
    int fd = this->fd_;
    this->fd_ = -1;

    return fd;
  }

  void reset(int fd = -1) noexcept {
    if (this->fd_ != -1 && fd != this->fd_) {
      close(this->fd_);
    }

    this->fd_ = fd;
  }

  int get() const noexcept {
    return this->fd_;
  }

private:
  int fd_;
};