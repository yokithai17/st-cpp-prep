#include "unique_fd.hpp"

#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <type_traits>
#include <utility>

static_assert(!std::is_copy_constructible_v<UniqueFd> &&
                  !std::is_copy_assignable_v<UniqueFd> &&
                  std::is_nothrow_move_constructible_v<UniqueFd> &&
                  std::is_nothrow_move_assignable_v<UniqueFd>,
              "UniqueFd must be move-only and nothrowable");

bool is_fd_open(int fd) {
  if (fd < 0) {
    return false;
  }
  return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

int test_empty() {
  UniqueFd empty;

  if (empty.get() != -1) {
    std::cerr << "Пустой объект должен возвращать -1\n";
    return 1;
  }

  return 0;
}

int test_destructor() {
  int pipefd[2];

  if (pipe(pipefd) == -1) {
    std::cerr << "Ошибка создания pipe\n";
    return 1;
  }

  {
    UniqueFd fd_r(pipefd[0]);
  }

  int res_r = fcntl(pipefd[0], F_GETFD);
  int errno_r = errno;

  close(pipefd[1]);
  int res_w = fcntl(pipefd[1], F_GETFD);
  int errno_w = errno;

  if (res_r == -1 && errno_r == EBADF && res_w == -1 && errno_w == EBADF) {
    return 0;
  }

  if (res_r >= 0) {
    close(pipefd[0]);
  }

  std::cerr << "Ошибка при закрытие pipe\n";

  return 1;
}

int test_move_constructor() {
  int pipefd[2];

  if (pipe(pipefd) == -1) {
    std::cerr << "Ошибка создания pipe\n";
    return 1;
  }

  int read_fd = pipefd[0];
  UniqueFd t(pipefd[1]);
  bool is_move_ok = false;

  {
    UniqueFd source(read_fd);
    UniqueFd destination(std::move(source));
    
    bool is_open_fd = is_fd_open(read_fd);
    is_move_ok = source.get() == -1 && destination.get() == read_fd && is_open_fd;
  }

  if (is_fd_open(read_fd)) {
    close(read_fd);
    std::cerr << "Ошибка in move-construct\n";
    return 1;    
  }

  if (!is_move_ok) {
    std::cerr << "Ошибка in move-construct\n";
    return 1; 
  }

  return 0;
}

int test_move_assign2() {
  int pipefd1[2];
  int pipefd2[2];

  if (pipe(pipefd1) == -1 || pipe(pipefd2) == -1) {
    std::cerr << "Ошибка создания pipe\n";
    return 1;
  }

  UniqueFd t1(pipefd1[1]);
  UniqueFd t2(pipefd2[1]);

  int read_fd1 = pipefd1[0];
  int read_fd2 = pipefd2[0];

  bool is_move_ok =  false;
  {
    UniqueFd source(read_fd1);
    UniqueFd destination(read_fd2);

    destination = std::move(source);

    bool is_open_r2 = is_fd_open(read_fd2);
    bool is_open_r1 = is_fd_open(read_fd1);
    is_move_ok = source.get() == -1 && !is_open_r2 && destination.get() == read_fd1 &&
        is_open_r1;
  }

  bool is_open_fd1 = is_fd_open(read_fd1);
  bool is_open_fd2 = is_fd_open(read_fd2);

  if (is_open_fd2 || is_open_fd1) {
    if (is_open_fd1) {
      close(read_fd1);
    }
    if (is_open_fd2) {
      close(read_fd2);
    }

    std::cerr << "Ошибка в move-assign2\n";
    return 1;
  }

  if (!is_move_ok) {
    return 1; 
  }

  return 0;
}

int test_move_assign1() {
  int pipefd[2];

  if (pipe(pipefd) == -1) {
    std::cerr << "Ошибка создания pipe\n";
    return 1;
  }

  int write_fd = pipefd[1];

  {
    UniqueFd source(pipefd[0]);
    UniqueFd destination;

    destination = std::move(source);

    if (source.get() == -1 && destination.get() == pipefd[0]) {
      close(write_fd);
      return 0;
    }
  }

  close(pipefd[0]);
  close(write_fd);

  std::cerr << "Ошибка в move-assign\n";

  return 1;
}

int test_move_assign() { return test_move_assign1() | test_move_assign2(); }

int test_release() {
  int pipefd[2];

  if (pipe(pipefd) == -1) {
    std::cerr << "Ошибка создания pipe\n";
    return 1;
  }

  int write_fd = pipefd[1];
  int read_fd = pipefd[0];
  int rel_fd = -1;

  {
    UniqueFd source(read_fd);
    int rel_fd = source.release();

    if (rel_fd != read_fd || source.get() != -1) {
      close(read_fd);
      close(write_fd);
      std::cerr << "Ошибка в release\n";
      return 1;
    }
  }

  bool is_still_open = is_fd_open(read_fd);

  close(read_fd);
  close(write_fd);

  if (!is_still_open) {
    std::cerr << "Ошибка в release : дискриптор закрылся после дестркутора\n";
    return 1;
  }

  return 0;
}

int test_reset3() {
  int pipefd[2];

  if (pipe(pipefd) == -1) {
    std::cerr << "Ошибка создания pipe\n";
    return 1;
  }

  int read_fd = pipefd[0];
  UniqueFd t(pipefd[1]);

  {
    UniqueFd source(read_fd);
    source.reset();

    if (source.get() != -1 || is_fd_open(read_fd)) {
      std::cerr << "Ошибка в reset(): дескриптор не сброшен в -1 или остался "
                   "открыт\n";
      return 1;
    }
  }

  return 0;
}

int test_reset2() {
  int pipefd[2];

  if (pipe(pipefd) == -1) {
    std::cerr << "Ошибка создания pipe\n";
    return 1;
  }

  int read_fd = pipefd[0];
  UniqueFd t(pipefd[1]);

  {
    UniqueFd source(read_fd);
    source.reset(source.get());

    if (source.get() != read_fd || !is_fd_open(read_fd)) {
      std::cerr << "Ошибка в reset(): дескриптор не сброшен в -1 или остался "
                   "открыт\n";
      return 1;
    }
  }

  return 0;
}

int test_reset1() {
  int pipefd1[2];
  int pipefd2[2];

  if (pipe(pipefd1) == -1 || pipe(pipefd2) == -1) {
    std::cerr << "Ошибка создания pipe\n";
    return 1;
  }

  int read_fd1 = pipefd1[0];
  int read_fd2 = pipefd2[0];

  UniqueFd t1(pipefd1[1]);
  UniqueFd t2(pipefd2[1]);

  {
    UniqueFd source(read_fd1);
    source.reset(read_fd2);

    if (source.get() != read_fd2 || is_fd_open(read_fd1) ||
        !is_fd_open(read_fd2)) {
      std::cerr << "Ошика в reset\n";
      return 1;
    }
  }

  return 0;
}

int test_reset() { return test_reset1() | test_reset2() | test_reset3(); }

int test_self_move() {
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    std::cout << "Ошибка создания pipe\n";
    return 1;
  }

  int read_fd = pipefd[0];
  int write_fd = pipefd[1];
  bool self_move_passed = false;

  {
    UniqueFd t(read_fd);
    t = std::move(t);

    if (t.get() == read_fd && is_fd_open(read_fd)) {
      self_move_passed = true;
    }
  }

  close(write_fd);

  if (!self_move_passed) {
    std::cout << "Ошибка в self move: объект повредил свои данные\n";
    return 1;
  }
  return 0;
}

int main() {
  return test_empty() | test_destructor() | test_move_constructor() |
         test_move_assign() | test_release() | test_reset() | test_self_move();
}