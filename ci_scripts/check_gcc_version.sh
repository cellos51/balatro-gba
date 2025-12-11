#!/bin/bash

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

check_gcc_version