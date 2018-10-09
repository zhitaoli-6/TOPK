#include "common.h"

#include <unistd.h>


#include <unordered_map>
#include <queue>
#include <vector>
#include <cstring>
#include <iostream>
#include <algorithm>

#include <libcuckoo/cuckoohash_map.hh>
//#define CUCKOO_HASH

//#include <thread>

typedef cuckoohash_map<std::string, int> cuckoo_table;
#define THREAD_NUM 4
#define BATCH_SIZE (1024*128)
//#include <map>

//#include <gperftools/profiler.h>


using namespace std;

struct timeval ts, te;


size_t getPeakRSS();
size_t getCurrentRSS();


static inline ull calcu_hash(char *str){
	ull x = 0;
	int i = 0;
	while(str[i]){
		x = x * 26 + str[i] - 'a';
		i++;
	}
	return x;
}

static int split_data(const char *path){
	FILE *raw_filp = fopen(path, "r+");
	if(raw_filp == NULL){
		perror("solve_topk fopen raw input");
		return errno;
	}

	FILE **sub_filp = new FILE*[SHARD_SIZE];
	char *sub_path = new char[1024];
	for(int i = 0; i < SHARD_SIZE; i++){
		sprintf(sub_path, "%s-sub-%d", path, i);
		sub_filp[i] = fopen(sub_path, "w");
		if(!sub_filp[i]) perror("split_data:");
		assert(sub_filp[i]);
	}

	//double hash_cost = 0;
	char *buf = new char[MAX_URL_LEN];
	while(fscanf(raw_filp, "%s", buf) != EOF){
		int shard = calcu_hash(buf) % SHARD_SIZE;
		//gettimeofday(&te, NULL);
		//hash_cost += TIME(ts, te);
		fprintf(sub_filp[shard], "%s\n", buf);
	}

	for(int i = 0; i < SHARD_SIZE; i++)
		fclose(sub_filp[i]);
	fclose(raw_filp);
	delete []sub_filp;
	delete []sub_path;
	delete []buf;
	//printf("%s: hash_cost %.2fs\n", __func__, hash_cost);
	return 0;
}

class str_cnt_pair_t{
	public:
		ull cnt;
		string str;
		friend bool operator< (const struct str_cnt_pair_t &can1, const struct str_cnt_pair_t &can2){
			return can1.cnt > can2.cnt;
		}
		str_cnt_pair_t(ull c, string s):cnt(c), str(s){}
};


static inline int batch_fscanf(FILE *filp, char *buf[], int MAX_ROW){
	int r = 0;
	while(fscanf(filp, "%s", buf[r]) != EOF){
		r++;
		if(r == MAX_ROW) break;
	}
	return r;
}



static int get_topk(const char *path, vector<str_cnt_pair_t> &topk_vec, int k){
	FILE **sub_filp = new FILE*[SHARD_SIZE];
	char *sub_path = new char[1024];
	//char *buf = new char[MAX_URL_LEN];
	char **buf = new  char*[BATCH_SIZE];
	for(int i = 0; i < BATCH_SIZE; i++)
		buf[i] = new char[MAX_URL_LEN];
#ifdef CUCKOO_HASH
	cuckoo_table str2cnt;
	//str2cnt.reserve(MIN_FILE_SIZE/1024/16);
	auto updatefn = [](int &num) { ++num; };
#else
	unordered_map<string, int> str2cnt;
#endif
	
	double hash_cost = 0;
	priority_queue<str_cnt_pair_t> cans_q; // candidate_priority_queue
	for(int i = 0; i < SHARD_SIZE; i++){
		sprintf(sub_path, "%s-sub-%d", path, i);
		sub_filp[i] = fopen(sub_path, "r");
		assert(sub_filp[i]);
		str2cnt.clear();
#ifdef CUCKOO_HASH
		int rows = 0;
		while((rows=batch_fscanf(sub_filp[i], buf, BATCH_SIZE))){
			gettimeofday(&ts, NULL);
#pragma omp parallel for
			for(int r = 0; r < rows; r++){
				str2cnt.upsert(string(buf[r]), updatefn, 1);
			}
			gettimeofday(&te, NULL);
			hash_cost += TIME(ts, te);
		}
		gettimeofday(&ts, NULL);
		
		auto lt = str2cnt.lock_table();
		for(const auto& it: lt){
			if(cans_q.size() < k) cans_q.push(str_cnt_pair_t(it.second, it.first));
			else if(k > 0 && cans_q.top().cnt < it.second){
				cans_q.pop();
				cans_q.push(str_cnt_pair_t(it.second, it.first));
			}
		}
		gettimeofday(&te, NULL);
		hash_cost += TIME(ts, te);
#else
		int rows = 0;
		while((rows=batch_fscanf(sub_filp[i], buf, BATCH_SIZE))){
			gettimeofday(&ts, NULL);

			for(int r = 0; r < rows; r++){
				str2cnt[string(buf[r])]++;
			}
			gettimeofday(&te, NULL);
			hash_cost += TIME(ts, te);
		}
		gettimeofday(&ts, NULL);
		for(auto& it: str2cnt){
			if(cans_q.size() < k) cans_q.push(str_cnt_pair_t(it.second, it.first));
			else if(k > 0 && cans_q.top().cnt < it.second){
				cans_q.pop();
				cans_q.push(str_cnt_pair_t(it.second, it.first));
			}
		}
		gettimeofday(&te, NULL);
		hash_cost += TIME(ts, te);
#endif
		/*
		   double load_factor = str2cnt.load_factor();
		   size_t size = str2cnt.size();
		   size_t bucket_count = str2cnt.bucket_count();
		   printf("sub_path str2cnt factor %.2f, size %lu,  bucket %lu\n", load_factor, size, bucket_count);
		   */
		fclose(sub_filp[i]);
	}
	printf("%s: hash_cost %.2fs\n", __func__, hash_cost);
	for(int i = 0; i < k; i++){
		topk_vec.push_back(cans_q.top());
		cans_q.pop();
	}
	delete []sub_filp;
	delete []sub_path;
	for(int i = 0; i < BATCH_SIZE; i++)
		delete []buf[i];
	delete buf;
	return 0;
}


static int solve_topk(const char *path, int k){
	struct timeval t1, t2;
	int ret = 0;
	gettimeofday(&t1, NULL);
	if((ret=split_data(path))) return ret;
	gettimeofday(&t2, NULL);

	double split_cost = TIME(t1, t2);

	vector<str_cnt_pair_t> topk_vec;
	//ProfilerStart("gperftools.topk");
	get_topk(path, topk_vec, k);
	//ProfilerStop();

	cout << "topk url: " << endl;
	for(int i = k-1; i >= 0; i--){
		cout << topk_vec[i].str << " " << topk_vec[i].cnt << endl;
	}
	gettimeofday(&t1, NULL);
	double cal_cost = TIME(t2, t1);
	printf("cost %.6fs, split %.6fs, get_topk %.6fs\n", split_cost + cal_cost, split_cost, cal_cost);
	return 0;
}

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
	printf("./solve_topk <scale>(1: 1G, 10:10G)\n");
}

int main(int argc, char *argv[]){
	//thread monitor(start_monitor);
	if(argc != 2){
		usage();
		return -1;
	}
	int scale = atoi(argv[1]);
	if(scale != 1 && scale != 10) {
		usage();
		return -1;
	}
	char path[256];
	sprintf(path, "/home/sirius/repos/test/pingcap/input/url%dG.in", scale);
	solve_topk(path, 3);
	//solve_topk("/home/sirius/repos/test/pingcap/input/tmp.in", 0);
	size_t rss = getPeakRSS();
	printf("memory used max: %.2fMB\n", 1.0*rss/(1024*1024) );
	return 0;
}
