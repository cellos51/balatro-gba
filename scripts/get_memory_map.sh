#!/usr/bin/env bash

set -euo pipefail

# Run this from the root directory of the project, not the scripts directory
POOL_DEF_FILE='./include/pools.def'
ELF_FILE='./build/balatro-gba.elf'
READELF='/opt/devkitpro/devkitARM/bin/arm-none-eabi-readelf'

get_pool_names() {
    grep POOL_ENTRY "$POOL_DEF_FILE" | sed -n 's@.*(\(.*\)).*@\1@p' | sed 's@,@@g' | cut -d ' ' -f 1
}

TOTAL_BYTES=0

echo "--------------------------------------------"
printf "%-16s| %-10s | %s\n" "Object" "location" "size"
echo "--------------------------------------------"

for name in $(get_pool_names); do
    output="$( \
            "$READELF" -sW "$ELF_FILE" | \
            grep "${name}_" | \
            grep OBJECT | \
            grep storage | sed -E 's@ +@ @g; s@^ @@' \
        )"

    location="$(cut -d ' ' -f 2 <<< $output)"
    size="$(cut -d ' ' -f 3 <<< $output)"
    TOTAL_BYTES=$(( TOTAL_BYTES + size ))

    printf "%-16s| 0x%s | %-8u\n" "$name" "$location" "$size"
done

echo "--------------------------------------------"
echo Total bytes used: $TOTAL_BYTES
