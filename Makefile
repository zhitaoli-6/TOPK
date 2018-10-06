all: regen_data solve_topk

CC = g++ 
CFLAGS = -std=c++11

regen_data: gen_data.cpp
	$(CC) $(CFLAGS) $(DEBUG) $< -o $@

solve_topk: topk.cpp
	$(CC) $(CFLAGS) $(DEBUG) $< -o $@



run:
	#./regen_data
	./solve_topk

clean:
	$(RM) input/*
	$(RM) regen_data solve_topk
