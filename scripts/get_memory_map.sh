#!/usr/bin/env bash

set -euo pipefail

POOL_DEF_FILE='./include/pools.def'
ELF_FILE="${1-./build/balatro-gba.elf}"
READELF='/opt/devkitpro/devkitARM/bin/arm-none-eabi-readelf'
TOTAL_BYTES=0

if [ ! -f "$ELF_FILE" ]; then
    echo "elf file not found: $ELF_FILE"
    echo "You can pass your elf file with:"
    echo "    $(basename $0) <file>"
    exit 1
fi

print_line_break() {
    echo "--------------------------------------------------------------------"
}

get_pool_names() {
    grep POOL_ENTRY "$POOL_DEF_FILE" | sed -n 's@.*(\(.*\)).*@\1@p' | sed 's@,@@g' | cut -d ' ' -f 1
}

print_line_break
printf "%-16s| %-10s | %-10s | %-10s | %-10s \n" "Object" "address" "pool size" "func size" "bitmap size"
print_line_break

for name in $(get_pool_names); do
    output_pool="$(                                  \
            "$READELF" -sW "$ELF_FILE"             | \
            grep "${name}_"                        | \
            grep OBJECT                            | \
            grep storage                           | \
            sed -E 's@ +@ @g; s@^ @@'                \
        )"
    output_func="$(                                  \
            "$READELF" -sW "$ELF_FILE"             | \
            grep -E "pool_free|pool_get|pool_init" | \
            grep -E "${name}$"                     | \
            sed -E 's@ +@ @g; s@^ @@'              | \
            tr -d '\n'                               \
        )"


    address="$(cut -d ' ' -f 2 <<< $output_pool)"
    pool_size="$(cut -d ' ' -f 3 <<< $output_pool)"
    func_size="$(cut -d ' ' -f 3 <<< $output_func)"
    bm_size=16 #always gonna be 16, 4 * sizeof(u32)
    
    TOTAL_BYTES=$(( TOTAL_BYTES + pool_size + func_size + bm_size ))

    printf "%-16s| 0x%8s | %-10u | %-10u | %-10u \n" "$name" "$address" "$pool_size" "$func_size" "$bm_size"
done

print_line_break
echo Total bytes used: $TOTAL_BYTES
