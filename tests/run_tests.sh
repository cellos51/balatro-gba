#!/bin/bash

set -eu

# Check GCC version and install GCC 14 if needed
check_gcc_version() {
    if command -v gcc &> /dev/null; then
        gcc_version=$(gcc -dumpversion | cut -d. -f1)
        if [ "$gcc_version" -lt 14 ]; then
            echo "GCC version $gcc_version detected. Installing GCC 14..."
            sudo apt-get update
            sudo apt-get install -y gcc-14
            sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100
            echo "GCC 14 installed and set as default."
        fi
    fi
}

run_test() {
    name="$1"

    echo "==============================================================================="
    echo "Running test for: $1"
    echo "==============================================================================="

    cd "$name" 2>&1 > /dev/null
    make clean > /dev/null
    make > /dev/null
    ./build/"$name"_test
    cd - 2>&1 > /dev/null
}

check_gcc_version

run_test bitset
run_test pool
run_test list
run_test util
