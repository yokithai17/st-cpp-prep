#include <atomic>
#include <cstdint>
#include <iostream>
#include <string>

static_assert(__cplusplus >= 202002L, "Build this target with C++20 or newer");

int main() {
    const std::atomic<std::uint64_t> counter{0};
    std::cout << "C++20 build and execution: OK\n"
              << "__cplusplus: " << __cplusplus << '\n'
              << "sizeof(void*): " << sizeof(void*) << '\n'
              << "sizeof(std::string): " << sizeof(std::string) << '\n'
              << "uint64_t atomic always lock-free: " << std::boolalpha
              << decltype(counter)::is_always_lock_free << '\n';
}
