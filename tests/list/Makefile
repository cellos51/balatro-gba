
CC := gcc
CFLAGS := -I../../include -I. \
          -g -O3 -Wall -Werror -DPOOLS_TEST_ENV=yes

SRC            := list_test.c          \
                  ../../source/list.c  \
                  ../../source/pool.c  \
                  ../../source/bitset.c
OUT            := build/list_test 

$(OUT): $(SRC) | build
	$(CC) $(CFLAGS) -o $@ $^ 

build:
	mkdir -p build

clean:
	rm -f $(OUT)
