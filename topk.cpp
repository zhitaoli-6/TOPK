#include "common.h"

#include <unistd.h>


#include <unordered_map>
#include <queue>
#include <cstring>
#include <iostream>
#include <algorithm>

#include <libcuckoo/cuckoohash_map.hh>
//#define CUCKOO_HASH

//#include <thread>

typedef cuckoohash_map<std::string, int> cuckoo_table;
//#include <map>

//#include <gperftools/profiler.h>


using namespace std;

struct timeval ts, te;



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
		// split each url according to its hash_code
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



// read MAX_ROW lines so we can run hash_map in parallel with 8 threads
static inline int batch_fscanf(FILE *filp, char *buf[], int MAX_ROW){
	int r = 0;
	while(fscanf(filp, "%s", buf[r]) != EOF){
		r++;
		if(r == MAX_ROW) break;
	}
	return r;
}



static int get_topk(const char *path, vector<str_cnt_pair_t> &topk_vec, const int k){
	FILE **sub_filp = new FILE*[SHARD_SIZE];
	char *sub_path = new char[1024];
	//char *buf = new char[MAX_URL_LEN];
	char **buf = new  char*[BATCH_SIZE];
	for(int i = 0; i < BATCH_SIZE; i++)
		buf[i] = new char[MAX_URL_LEN];
	/* 
	 * Priority_queue(Min_heap) with size k  is used to maintain topk most frequent urls.
	 * The time complexity is O(N*logK), where N is the number of elements
	 */
	priority_queue<str_cnt_pair_t> cans_q; 
#ifdef CUCKOO_HASH
	// concurrent hash_map(cuckoohash_map) is used to speedup hash_map operations
	cuckoo_table str2cnt;
	//str2cnt.reserve(MIN_FILE_SIZE/1024/16);
	// lambda function to update value of given key
	auto updatefn = [](int &num) { ++num; };
#else
	unordered_map<string, int> str2cnt;
#endif
	
	double hash_cost = 0;
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
				// If the number is already in the table, it will increment
				// its count by one. Otherwise it will insert a new entry in
				// the table with count one.
				str2cnt.upsert(string(buf[r]), updatefn, 1);
			}
			gettimeofday(&te, NULL);
			hash_cost += TIME(ts, te);
		}
		gettimeofday(&ts, NULL);

		// iterate all entry in hash_map to get topk keys with biggest values
		auto lt = str2cnt.lock_table();
		for(const auto& it: lt){
			// min_heap current size less than k, insert it into heap directly
			if(cans_q.size() < k) cans_q.push(str_cnt_pair_t(it.second, it.first));
			// if min_heap is full(size k) and current entry bigger than minimal value in priority, insert it into heap
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
		// iterate all entry in hash_map to get topk keys with biggest values
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
#ifdef DEBUG
	printf("%s: hash_cost %.2fs\n", __func__, hash_cost);
#endif
	// traverse min_heap to store topk key-value pairs into result
	int total_entry = cans_q.size();
	if(total_entry != k){
		printf("WARNING: expect %d entries while found only %d entries\n", k, total_entry);
	}
	for(size_t i = 0; i < total_entry; i++){
		topk_vec.push_back(cans_q.top());
		cans_q.pop();
	}
	//printf("topk solution find %lu entry, %lu\n", topk_vec.size(), cans_q.size());
	delete []sub_filp;
	delete []sub_path;
	for(int i = 0; i < BATCH_SIZE; i++)
		delete []buf[i];
	delete buf;
	return 0;
}


int  solve_topk(const char *path, vector<str_cnt_pair_t> &topk_vec, int k){
	struct timeval t1, t2;
	int ret = 0;
	gettimeofday(&t1, NULL);
	if((ret=split_data(path))) return ret;
	gettimeofday(&t2, NULL);

	double split_cost = TIME(t1, t2);

	//ProfilerStart("gperftools.topk");
	get_topk(path, topk_vec, k);
	//ProfilerStop();

	//reverse(topk_vec.begin(), topk_vec.end());
	gettimeofday(&t1, NULL);
	double cal_cost = TIME(t2, t1);
#ifdef DEBUG
	printf("cost %.6fs, split %.6fs, get_topk %.6fs\n", split_cost + cal_cost, split_cost, cal_cost);
#endif
	return 0;
}


