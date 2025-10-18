POOLS_DEF_FILE := \"test_pools.def\"

CC := gcc
CFLAGS := -I../../include -I. \
          -g -O3 -Wall -Werror -DPOOLS_DEF_FILE=$(POOLS_DEF_FILE) 

SRC            := pool_test.c ../../source/pool.c
OUT            := build/pool_test

$(OUT): $(SRC) | build
	$(CC) $(CFLAGS) -o $@ $^ 

build:
	mkdir -p build

clean:
	rm -f $(OUT)
