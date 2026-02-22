#!/bin/bash
set -euo pipefail

usage() {
    echo "Usage: $0 [-f output_file]" >&2
}

output_file=""
while getopts ":f:" opt; do
    case "$opt" in
        f) output_file="$OPTARG" ;;
        \?) usage; exit 1 ;;
        :) echo "Option -$OPTARG requires an argument." >&2; usage; exit 1 ;;
    esac
done

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

run_cmd cmake -S . -B build -DBUILD_TESTS=OFF
run_cmd cmake --build build
