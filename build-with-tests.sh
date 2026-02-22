#!/bin/bash
cmake -S . -B build-tests -DBUILD_TESTS=ON
cmake --build build-tests
ctest --test-dir build-tests $@
