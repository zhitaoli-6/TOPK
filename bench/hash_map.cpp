#include <unordered_map>
#include <iostream>
#include <algorithm>
#include "common.h"
using namespace std;
#define MAXN 200000

string str[MAXN];


void init(){
	char *buf = new char[MAX_URL_LEN];
	for(int i = 0; i < MAXN; i++){
		int len = MIN_URL_LEN + rand() % (MAX_URL_LEN - MIN_URL_LEN);
		for(int j = 0; j < len; j++)
			buf[j] = rand() % 26 + 'a';
		buf[len] = '\0';
		str[i] = string(buf);
	}
	delete []buf;
}

int main(){
	init();
	unordered_map<string, int> str2cnt(MAXN+10);
	//str2cnt.max_load_factor(100);
	//str2cnt.max_load_factor(100);
	struct timeval t1, t2, t3;
	gettimeofday(&t1, NULL);
	for(int i = 0; i < MAXN; i++){
		str2cnt[str[i]]++;
	}
	gettimeofday(&t2, NULL);
	int mx = -1;
	string buf("hello");
	for(auto &it: str2cnt){
		if(it.second > mx) {
			mx = it.second;
			buf = it.first;
		}
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
