all: bench_hash_map bench_io

CC = g++
IFLAGS = -I..
CFLAGS = -std=c++11 -O3
#LDFLAGS = -lprofiler



bench_hash_map: hash_map.cpp
	$(CC) $(CFLAGS) $(IFLAGS) $(DEBUG) $< -o $@

bench_io: io.cpp
	$(CC) $(CFLAGS) $(IFLAGS) $(DEBUG) $< ../mem-stat.o -o $@

clean:
	$(RM) bench_hash_map bench_io
