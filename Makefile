all: mem-stat.o gen_data.o topk.o solve_topk self_test

CC = g++
CFLAGS = -std=c++11 -O3 -fopenmp
#LDFLAGS = -lprofiler


mem-stat.o: mem-stat.cpp common.h
	$(CC) -c $<

gen_data.o: gen_data.cpp common.h
	$(CC) -c $<

topk.o: topk.cpp common.h
	$(CC) $(CFLAGS) -I/home/sirius/repos/libcuckoo/install/include -c $<


solve_topk: solver.cpp topk.cpp gen_data.cpp mem-stat.cpp common.h
	$(CC) $(CFLAGS) solver.cpp -o $@ mem-stat.o gen_data.o topk.o $(LDFLAGS)

self_test: self_test.cpp topk.cpp  mem-stat.cpp common.h
	$(CC) $(CFLAGS) self_test.cpp -o $@ mem-stat.o  topk.o $(LDFLAGS)



run:
	$(RM) input/*-sub-*
	./solve_topk

clean:
	#$(RM) input/*
	$(RM) solve_topk mem-stat.o topk.o gen_data.o self_test
