"""Отдельные положительные и отрицательные проверки компиляции C++20."""

import argparse
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cxx", default="g++")
    args = parser.parse_args()
    include = Path(__file__).resolve().parents[1] / "exercises/03-ring-buffer"
    prefix = '''#include "ring_buffer.hpp"
#include <memory>
#include <string>
struct NoDefault { explicit NoDefault(int); NoDefault(NoDefault&&) noexcept; };
struct BadMove { BadMove(BadMove&&) noexcept(false); };
struct BadDtor { ~BadDtor() noexcept(false); };
'''
    cases = [("int", True), ("std::string", True),
             ("std::unique_ptr<int>", True), ("NoDefault", True),
             ("const int", False), ("volatile int", False),
             ("int[2]", False), ("int&", False), ("void", False),
             ("void()", False), ("BadMove", False), ("BadDtor", False)]
    with tempfile.TemporaryDirectory(prefix="ring-types-") as directory:
        source = Path(directory) / "check.cpp"
        for cpp_type, accepted in cases:
            source.write_text(prefix + f"RingBuffer<{cpp_type}> buffer(1);\n")
            result = subprocess.run(
                [args.cxx, "-std=c++20", "-Wall", "-Wextra", "-Wpedantic",
                 "-Werror", "-fsyntax-only", "-I", str(include), str(source)],
                capture_output=True, text=True)
            correct = ((result.returncode == 0) == accepted)
            if not accepted:
                correct = correct and "static assertion failed" in result.stderr
            if not correct:
                raise SystemExit(f"Unexpected result for {cpp_type}:\n{result.stderr}")
            print(f"PASS {cpp_type}: {'accepted' if accepted else 'rejected'}")


if __name__ == "__main__":
    main()
