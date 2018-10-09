#include <unordered_map>
#include <iostream>
#include <vector>
#include <algorithm>
#include <omp.h>
#include "common.h"
using namespace std;
#define MAXN 1000000

#include <libcuckoo/cuckoohash_map.hh>

//#define SMP

#define THREAD_NUM 8

string str[MAXN];


void init(){
	char *buf = new char[MAX_URL_LEN];
	for(int i = 0; i < MAXN; i++){
		int len = 2 + rand() % 5;
		for(int j = 0; j < len; j++)
			buf[j] = rand() % 26 + 'a';
		buf[len] = '\0';
		str[i] = string(buf);
	}
	delete []buf;
}

typedef cuckoohash_map<std::string, int> smp_table;

void do_inserts(smp_table &freq_map, size_t tid) {
	printf("thread %lu starts\n", tid);
	size_t shard_size = MAXN / THREAD_NUM;
	size_t start = tid * shard_size;
	size_t end = start + shard_size;
	if(end >= MAXN) end = MAXN;
	auto updatefn = [](int &num) { ++num; };
	for (size_t i = start; i < end; i++) {
		// If the number is already in the table, it will increment
		// its count by one. Otherwise it will insert a new entry in
		// the table with count one.
		freq_map.upsert(str[i], updatefn, 1);
	}
}



int main(){
	init();
	struct timeval t1, t2, t3;
	gettimeofday(&t1, NULL);
#ifdef SMP
	printf("SMP mode\n");
	smp_table str2cnt;
	str2cnt.reserve(MAXN);

	/*
	auto updatefn = [](int &num) { ++num; };
#pragma omp parallel
#pragma omp master
	{
		printf("total threads %u\n", omp_get_num_threads());
	}
#pragma omp parallel for num_threads(8)
	for(int i = 0; i < MAXN; i++)
		str2cnt.upsert(str[i], updatefn, 1);
		*/

	vector<thread> threads;
	for (size_t i = 0; i < THREAD_NUM; i++) {
		threads.emplace_back(do_inserts, std::ref(str2cnt), i);
	}
	for (size_t i = 0; i < THREAD_NUM; i++) {
		threads[i].join();
	}
#else
	printf("built-in mode\n");
	//unordered_map<string, int> str2cnt;
	unordered_map<string, int> str2cnt(MAXN);
	//str2cnt.max_load_factor(100);
	for(int i = 0; i < MAXN; i++){
		str2cnt[str[i]]++;
	}
#endif
	gettimeofday(&t2, NULL);
	int mx = -1;
	string buf("hello");
#ifdef SMP
	auto lt = str2cnt.lock_table();
	for(const auto &it: lt){
		if(it.second > mx) {
			mx = it.second;
			buf = it.first;
		}
#else
	for(const auto &it: str2cnt){
		if(it.second > mx) {
			mx = it.second;
			buf = it.first;
		}
#endif
	}
	gettimeofday(&t3, NULL);
	cout << mx << "  " << buf << endl;
	cout << "init hash_map " << TIME(t1, t2) << endl;
	cout << "iter hash_map " << TIME(t2, t3) << endl;
	double load_factor = str2cnt.load_factor();
	size_t size = str2cnt.size();
	size_t bucket_count = str2cnt.bucket_count();
	printf(" str2cnt factor %.2f, size %lu,  bucket %lu\n", load_factor, size, bucket_count);
	return 0;
}
