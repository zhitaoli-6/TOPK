#include "common.h"

#include <unordered_map>
#include <queue>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

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
		assert(sub_filp[i]);
	}

	char *buf = new char[MAX_URL_LEN];
	while(fscanf(raw_filp, "%s", buf) != EOF){
		int shard = calcu_hash(buf) % SHARD_SIZE;
		fprintf(sub_filp[shard], "%s\n", buf);
	}

	for(int i = 0; i < SHARD_SIZE; i++)
		fclose(sub_filp[i]);
	fclose(raw_filp);
	delete []sub_filp;
	delete []sub_path;
	delete []buf;
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


static int solve_topk(const char *path, int k){
	struct timeval t1, t2;
	int ret = 0;
	gettimeofday(&t1, NULL);
	if((ret=split_data(path))) return ret;
	gettimeofday(&t2, NULL);
	
	double split_cost = TIME(t1, t2);
	

	unordered_map<string, int> str2cnt;
	FILE **sub_filp = new FILE*[SHARD_SIZE];
	char *sub_path = new char[1024];
	char *buf = new char[MAX_URL_LEN];
	priority_queue<str_cnt_pair_t> cans_q;
	for(int i = 0; i < SHARD_SIZE; i++){
		sprintf(sub_path, "%s-sub-%d", path, i);
		sub_filp[i] = fopen(sub_path, "r");
		assert(sub_filp[i]);
		//str2cnt.clear();
		while(fscanf(sub_filp[i], "%s", buf) != EOF){
			//str2cnt[string(buf)]++;
		}
		/*
		for(auto& it: str2cnt){
			if(cans_q.size() < k) cans_q.push(str_cnt_pair_t(it.second, it.first));
			else if(cans_q.top().cnt < it.second){
				cans_q.pop();
				cans_q.push(str_cnt_pair_t(it.second, it.first));
			}
		}
		*/
		fclose(sub_filp[i]);
	}
	vector<str_cnt_pair_t> topk_can;
	for(int i = 0; i < k; i++){
		topk_can.push_back(cans_q.top());
		cans_q.pop();
	}
	cout << "topk url: " << endl;
	for(int i = k-1; i >= 0; i--){
		cout << topk_can[i].str << " " << topk_can[i].cnt << endl;
	}
	gettimeofday(&t1, NULL);
	double cal_cost = TIME(t2, t1);
	printf("cost %.6fs, split %.6fs, cal %.6fs\n", split_cost + cal_cost, split_cost, cal_cost);
	return 0;
}

int main(int argc, char *argv[]){
	solve_topk("/home/sirius/repos/test/pingcap/input/url1G.in", 0);
	//solve_topk("/home/sirius/repos/test/pingcap/input/tmp.in", 100);
	return 0;
}
