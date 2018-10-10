#include "common.h"

#include <thread>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unistd.h>
using namespace std;


extern size_t getPeakRSS();
extern size_t getCurrentRSS();

extern void gen_data(const char *path, const ull size);
extern int  solve_topk(const char *path, vector<str_cnt_pair_t> &topk_vec, int k);

static void start_monitor(){
	size_t crss, mrss;
	while(1){
		crss = getCurrentRSS();
		mrss = getPeakRSS();
		printf("momory_monitor: cur rss %.2fMB, max rss %.2fMB\n", 1.0*crss/(1024*1024), 1.0*mrss/(1024*1024));
		sleep(1);
	}
}

static void usage(){
	printf("./solve_topk <gen> <scale>\n");
	printf("<gen>: whether to generate input. 0:no, 1:yes\n");
	printf("<scale>: workload, 1:1GB, 10:10GB\n");
}

int main(int argc, char *argv[]){
	//thread monitor(start_monitor);
	if(argc != 3){
		usage();
		return -1;
	}
	int gen = atoi(argv[1]);
	assert(gen ==  0 || gen == 1);
	int scale = atoi(argv[2]);
	assert(scale ==  1 || scale == 10);
	char path[256];
	sprintf(path, "./input/url%dG.in", scale);
	ull size = 1ull * scale * MIN_FILE_SIZE;
	vector<str_cnt_pair_t> topk_vec;
	if(gen){
		gen_data(path, size);
	}
	int k = 100;
	solve_topk(path, topk_vec, k);
	cout << "topk url: " << endl;
	int found_k = topk_vec.size();
	for(int i = found_k-1; i >= 0; i--){
		cout << topk_vec[i].str << " " << topk_vec[i].cnt << endl;
	}
	//solve_topk("/home/sirius/repos/test/pingcap/input/tmp.in", 0);
	size_t rss = getPeakRSS();
	printf("memory used max: %.2fMB\n", 1.0*rss/(1024*1024) );
	return 0;
}
