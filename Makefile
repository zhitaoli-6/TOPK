all: regen_data solve_topk mem-stat.o

CC = g++ -fopenmp
CFLAGS = -std=c++11 -O3
#LDFLAGS = -lprofiler


mem-stat.o: mem-stat.cpp
	$(CC) -c $<

regen_data: gen_data.cpp common.h
	$(CC) $(CFLAGS) $(DEBUG) $< -o $@

solve_topk: topk.cpp mem-stat.o common.h
	$(CC) $(CFLAGS) $(DEBUG) -I/home/sirius/repos/libcuckoo/install/include topk.cpp -o $@ mem-stat.o $(LDFLAGS)



run:
	#./regen_data
	$(RM) input/*-sub-*
	./solve_topk

clean:
	#$(RM) input/*
	$(RM) regen_data solve_topk mem-stat.o
