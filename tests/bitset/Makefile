
CC := gcc
CFLAGS := -I../../include -I. \
          -g -O3 -Wall -Werror
SRC            := bitset_test.c         \
                  ../../source/bitset.c
OUT            := build/bitset_test 

$(OUT): $(SRC) | build
	$(CC) $(CFLAGS) -o $@ $^ 

build:
	mkdir -p build

clean:
	rm -f $(OUT)
