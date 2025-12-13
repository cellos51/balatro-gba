
CC := gcc
CFLAGS := -I../../include -I. \
          -g -O3 -std=gnu23 -Wall -Werror -DPOOLS_TEST_ENV=yes

SRC            := pool_test.c ../../source/pool.c ../../source/bitset.c
OUT            := build/pool_test 

$(OUT): $(SRC) | build
	$(CC) $(CFLAGS) -o $@ $^ 

build:
	mkdir -p build

clean:
	rm -f $(OUT)
