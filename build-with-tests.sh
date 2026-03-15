#!/bin/bash
set -euo pipefail

# Default to online FetchContent behavior so GoogleTest can be downloaded.
# Allow callers to override, e.g. FETCHCONTENT_FULLY_DISCONNECTED=ON ./build-with-tests.sh
fetchcontent_disconnected="${FETCHCONTENT_FULLY_DISCONNECTED:-OFF}"

cmake -S . -B build-tests \
    -DBUILD_TESTS=ON \
    -DFETCHCONTENT_FULLY_DISCONNECTED="${fetchcontent_disconnected}"
cmake --build build-tests
cd build-tests
ctest "$@"
