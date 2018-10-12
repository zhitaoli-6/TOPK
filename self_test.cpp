/*
 * self_test.cpp: black-box test for topk solution, which we call fast-topk.
 *
 * Simply, naive solution, which we call ref-topk, assumes memory is not limited and uses hash_map to calculate the number of every item.
 * Then, many workloads of different scales are generated. We run two fast-topk and ref-topk, then results are compared to make sure they are consistent with each other.
 */


#include "common.h"

#include <thread>
#include <vector>
#include <map>
#include <unistd.h>
#include <iostream>
#include <algorithm>
using namespace std;


extern size_t getPeakRSS();
extern size_t getCurrentRSS();

extern void gen_data(const char *path, const ull size);
extern int  solve_topk(const char *path, const int mode, vector<str_cnt_pair_t> &topk_vec, int k);

#define MAX_TEST_ROW 1000000

char str[MAX_TEST_ROW][MAX_URL_LEN];


void gen_skew_data(int r){
	//assert(r >= 100);
	for(int i = 0; i < r; i++){
		int len = 2 + rand()%3;
		for(int j = 0; j < len; j++){
			int tmp = rand() % 4;
			if(tmp == 0) str[i][j] = 'a';
			else str[i][j] = rand()%7 + 'a';
		}
		str[i][len] = '\0';
	}
}

void write_to_file(const char* path, int r){
	FILE *filp = fopen(path, "w");
	assert(filp);
	for(int i = 0; i < r; i++){
		fprintf(filp, "%s\n", str[i]);
	}
	fclose(filp);
}

void print_data(int row){
	cout << "-----------" << endl;
	for(int i = 0; i < row; i++)
		cout << str[i] << endl;
	cout << "-----------" << endl;
}
void print_solution(vector<str_cnt_pair_t> &topk_vec, int k){
	printf("----solver topk---\n");
	int t = topk_vec.size();
	cout << k << " " << topk_vec.size() << endl;
	for(int j = t-1; j>=0; j--)
		cout << topk_vec[j].str << ":" << topk_vec[j].cnt << " ";
	cout << endl;
}
void print_ref(vector<str_cnt_pair_t> &all_pair){
	printf("----ref ---\n");
	int t = all_pair.size();
	for(int j = 0; j < t; j++)
		cout << all_pair[j].str << ":" << all_pair[j].cnt << " ";
	cout << endl;
}


int main(int argc, char *argv[]){
	srand(time(0));
	char path[256];
	map<string, int> str2cnt;
	map<int, int> cnt2len;
	//priority_queue<str_cnt_pair_t> min_heap;
	int k = 100;
	for(int row = 10; row <= MAX_TEST_ROW; row *= 10){
		str2cnt.clear();
		//cnt2len.clear();
		sprintf(path, "./input/self-test-sample");
		gen_skew_data(row);
	//	print_data(row);
		write_to_file(path, row);
		vector<str_cnt_pair_t> topk_vec;
		vector<str_cnt_pair_t> all_s2c;
		if(solve_topk(path, 0, topk_vec, k) == 0){
			//printf("solution runs ok on rows %d\n", row);


			for(int i = 0; i < row; i++)
				str2cnt[string(str[i])]++;
			for(auto &it: str2cnt){
				all_s2c.push_back(str_cnt_pair_t(it.second, it.first));
			}
			sort(all_s2c.begin(), all_s2c.end());
			int t = topk_vec.size();
			if(t < k && k <= all_s2c.size()) {
				goto error;
			}
			// check soution works consistently with ref answers
			for(int i = t-1; i >= 0; i--){
				string ref_str = all_s2c[t-1-i].str;
				int ref_cnt = all_s2c[t-1-i].cnt;
				string cur_str = topk_vec[i].str;
				int cur_cnt = topk_vec[i].cnt;
				//cout << "check " << cur_str << ":" << cur_cnt << endl;
				if(ref_cnt == cur_cnt && str2cnt[cur_str] == ref_cnt)
					continue;
				goto error;
			}
			printf("right on %d rows workload\n", row);
			printf("----------------\n");
			continue;
error:
			printf("error on row %d workload\n", row);
			print_ref(all_s2c);
			print_solution(topk_vec, k);
			return -1;
		}
		else{
			printf("error when run solve_topk on rows %d\n", row);
			return -1;
		}
	}
	return 0;
}
