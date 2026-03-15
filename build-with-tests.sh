#!/bin/bash
set -euo pipefail

usage() {
    echo "Usage: $0 [-c gcc|clang] [ctest args...]" >&2
}

# Default to online FetchContent behavior so GoogleTest can be downloaded.
# Allow callers to override, e.g. FETCHCONTENT_FULLY_DISCONNECTED=ON ./build-with-tests.sh
fetchcontent_disconnected="${FETCHCONTENT_FULLY_DISCONNECTED:-OFF}"

compiler=""
while getopts ":c:" opt; do
    case "$opt" in
        c) compiler="$OPTARG" ;;
        \?) usage; exit 1 ;;
        :) echo "Option -$OPTARG requires an argument." >&2; usage; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

build_dir="build-tests"
case "$compiler" in
    "")
        ;;
    gcc)
        export CC="${CC:-gcc}"
        export CXX="${CXX:-g++}"
        build_dir="build-tests-gcc"
        ;;
    clang)
        export CC="${CC:-clang}"
        export CXX="${CXX:-clang++}"
        build_dir="build-tests-clang"
        ;;
    *)
        echo "Unsupported compiler: $compiler" >&2
        usage
        exit 1
        ;;
esac

cmake_args=(-S . -B "$build_dir" -DBUILD_TESTS=ON
    -DFETCHCONTENT_FULLY_DISCONNECTED="${fetchcontent_disconnected}")
if [[ -n "${CC:-}" ]]; then
    cmake_args+=("-DCMAKE_C_COMPILER=${CC}")
fi
if [[ -n "${CXX:-}" ]]; then
    cmake_args+=("-DCMAKE_CXX_COMPILER=${CXX}")
fi

cmake "${cmake_args[@]}"
cmake --build "$build_dir"
cd "$build_dir"
ctest "$@"
