#!/usr/bin/env bash

set -euo pipefail

ELF_FILE="${ELF_FILE-./build/balatro-gba.elf}"
POOL_DEF_FILE="${POOL_DEF_FILE-./include/def_balatro_mempool.h}"
READELF="${READELF-/opt/devkitpro/devkitARM/bin/arm-none-eabi-readelf}"
TOTAL_BYTES=0

if [ ! -f "$POOL_DEF_FILE" ]; then
    echo "Mempool definition file not found: $POOL_DEF_FILE"
    echo "You can set your mempool definition file with:"
    echo "    POOL_DEF_FILE=<mempool-def-file> $(basename $0)"
    exit 1
fi

if [ ! -f "$ELF_FILE" ]; then
    echo "elf file not found: $ELF_FILE"
    echo "You can set your elf file with:"
    echo "    ELF_FILE=<elf-file> $(basename $0)"
    exit 1
fi

if [ ! -x "$READELF" ]; then
    echo "ERROR: \"$READELF\" is not an executable file."
    echo "You can override the file location for 'arm-none-eabi-readelf' with the READELF env variable."
    echo "  e.g. $ READELF=\"/my/custom/location/arm-none-eabi-readelf\" $(basename $0) <file>"
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
    output_bitset="$(                                \
            "$READELF" -sW "$ELF_FILE"             | \
            grep -E "${name}_bitset_w"             | \
            grep OBJECT                            | \
            sed -E 's@ +@ @g; s@^ @@'              | \
            tr -d '\n'                               \
        )"

    address="$(cut -d ' ' -f 2 <<< $output_pool)"
    pool_size="$(cut -d ' ' -f 3 <<< $output_pool)"
    func_size="$(cut -d ' ' -f 3 <<< $output_func)"
    bitset_size="$(cut -d ' ' -f 3 <<< $output_bitset)"
    
    TOTAL_BYTES=$(( TOTAL_BYTES + pool_size + func_size + bitset_size ))

    printf "%-16s| 0x%8s | %-10u | %-10u | %-10u \n" "$name" "$address" "$pool_size" "$func_size" "$bitset_size"
done

print_line_break
echo Total bytes used: $TOTAL_BYTES
