/* Solver.cpp: high-level modules to solve topk problem
 *
 * 1. gen_data
 * 2. topk solution. please see topk.cpp in detail.
 * 3. output topk hottest urls
 */

#include "common.h"

#include <thread>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unistd.h>
using namespace std;


extern size_t getPeakRSS();
extern size_t getCurrentRSS();

extern void gen_data(const char *path, const ull size, const int min_url_len=MIN_URL_LEN, const int max_url_len=MAX_URL_LEN);
extern int  solve_topk(const char *path, const int mode, vector<str_cnt_pair_t> &topk_vec, int k);

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
	printf("./solve_topk <gen> <real_url> <scale>\n");
	printf("<gen>: whether to generate input. 0:no, 1:yes\n");
	printf("<real_url>: whether url len:60-120. 0:no, 1:yes\n");
	printf("<scale>: workload, 1:1GB, 10:10GB\n");
}

int main(int argc, char *argv[]){
	//thread monitor(start_monitor);
	if(argc != 4){
		usage();
		return -1;
	}
	int gen = atoi(argv[1]);
	assert(gen ==  0 || gen == 1);
	int real_url = atoi(argv[2]);
	assert(real_url == 0 || real_url == 1);
	int scale = atoi(argv[3]);
	assert(scale ==  1 || scale == 10);
	char path[256];
	sprintf(path, "./input/url%dG%s.in", scale, real_url?"real":"");
	ull size = 1ull * scale * MIN_FILE_SIZE;
	vector<str_cnt_pair_t> topk_vec;
	if(gen){
		if(!real_url)
			gen_data(path, size);
		else
			gen_data(path, size, REAL_MIN_URL_LEN, REAL_MAX_URL_LEN);
	}
	int k = 3;
	solve_topk(path, real_url, topk_vec, k);
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
