#include <thread>
#include <vector>
#include "common.h"
using namespace std;
#define ITER 1024

void callee(){

}

int main(){
	struct timeval t1, t2;
	gettimeofday(&t1, NULL);
	for(int i = 0; i < 1; i++){
		vector<thread> threads;
		for (size_t i = 0; i < ITER; i++) {
			threads.emplace_back(callee);
		}
		for (size_t i = 0; i < ITER; i++) {
			threads[i].join();
		}
	}
	gettimeofday(&t2, NULL);
	printf("%.6fs\n", TIME(t1, t2));
	return 0;
}
