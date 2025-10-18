#!/bin/bash

set -eu

run_pool_test() {
    cd pool
    make clean
    make
    ./build/pool_test
    cd - > /dev/null 
}

run_pool_test
