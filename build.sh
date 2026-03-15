#!/bin/bash
set -euo pipefail

usage() {
    echo "Usage: $0 [-c gcc|clang] [-f output_file]" >&2
}

output_file=""
compiler=""
while getopts ":c:f:" opt; do
    case "$opt" in
        c) compiler="$OPTARG" ;;
        f) output_file="$OPTARG" ;;
        \?) usage; exit 1 ;;
        :) echo "Option -$OPTARG requires an argument." >&2; usage; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

build_dir="build"
case "$compiler" in
    "")
        ;;
    gcc)
        export CC="${CC:-gcc}"
        export CXX="${CXX:-g++}"
        build_dir="build-gcc"
        ;;
    clang)
        export CC="${CC:-clang}"
        export CXX="${CXX:-clang++}"
        build_dir="build-clang"
        ;;
    *)
        echo "Unsupported compiler: $compiler" >&2
        usage
        exit 1
        ;;
esac

run_cmd() {
    if [[ -n "$output_file" ]]; then
        "$@" 2>&1 | tee -a "$output_file"
    else
        "$@"
    fi
}

if [[ -n "$output_file" ]]; then
    : > "$output_file"
fi

cmake_args=(-S . -B "$build_dir" -DBUILD_TESTS=OFF)
if [[ -n "${CC:-}" ]]; then
    cmake_args+=("-DCMAKE_C_COMPILER=${CC}")
fi
if [[ -n "${CXX:-}" ]]; then
    cmake_args+=("-DCMAKE_CXX_COMPILER=${CXX}")
fi

run_cmd cmake "${cmake_args[@]}"
run_cmd cmake --build "$build_dir"
