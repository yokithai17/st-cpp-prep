# Запуск проверок

Тесты студента находятся в `test.cpp`, вспомогательные типы — в
`test_class.hpp`. По прямой просьбе студента наставник дописал функции
`test_buffer_exceptions`, `test_edges_and_references`, `test_model`
и управление отказом обычного конструктора тестового типа.

Из корня репозитория:

```bash
cmake -S . -B build-gcc -DCMAKE_BUILD_TYPE=Debug
cmake --build build-gcc --target ring_buffer_tests
ctest --test-dir build-gcc -R '^ring_buffer_tests$' --output-on-failure
python3 tools/check_ring_buffer_types.py
```

Тесты используют `assert`, поэтому требуют сборку без `NDEBUG`.
Отрицательные проверки компиляции запускаются отдельно; инструмент проверяет
успех допустимых типов и диагностику static assertion для неподдерживаемых.
Он не входит в обычную успешную C++-сборку. Другой компилятор выбирается
аргументом `--cxx clang++`.

Санитайзеры:

```bash
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -g \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  exercises/03-ring-buffer/test.cpp -o /tmp/st-ring-sanitized
/tmp/st-ring-sanitized
```

В модели используется `std::deque<int>`, seed `0xC0FFEE`, по 3000 операций
для вместимостей 0, 1, 2, 3, 7. При несовпадении печатаются seed, вместимость
и история операций; после последовательности сравнивается оставшийся FIFO.
Санитайзеры и модель не доказывают отсутствие всех ошибок.
