#include "common.h"

#include <unistd.h>

#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>


using namespace std;


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


static int read_one_file(const char *path){
	struct timeval t1, t2;
	gettimeofday(&t1, NULL);
	int fd = open(path, O_RDONLY);
	if(fd == -1){
		perror("error read file");
		return -1;
	}
	char *buf = new char[MIN_FILE_SIZE];
	memset(buf, 0, MIN_FILE_SIZE);
	ull count = read(fd, buf, MIN_FILE_SIZE);
	ull row = 0;
	for(ull i = 0; i < count; i++) 
		if(buf[i] == '\n') row++;
	gettimeofday(&t2, NULL);
	//printf("%s: read %llu bytes take %.2fs\n", __func__, count, TIME(t1, t2));
	printf("%s: read and count %llu bytes take %.2fs, rows %llu\n", __func__, count, TIME(t1, t2), row);
	close(fd);
	/*
	ull x=0;
	for(int i = 0; i < count; i++)
		x += buf[i] - 'a';
	printf("magic %llu\n", x);
	*/
}

static int fscanf_one_file(const char *path){
	FILE *raw_filp = fopen(path, "r");
	if(raw_filp == NULL){
		perror("solve_topk fopen raw input");
		return errno;
	}

	struct timeval t1, t2;

	gettimeofday(&t1, NULL);
	char *buf = new char[MAX_URL_LEN];
	while(fscanf(raw_filp, "%s", buf) != EOF){
	}
	gettimeofday(&t2, NULL);
	double read_cost = TIME(t1, t2);
	//printf("%s: hash_cost %.2fs\n", __func__, hash_cost);
	printf("%s: read raw_input %.2fs\n", __func__, read_cost);
	fclose(raw_filp);
	delete []buf;
	return 0;

}


static int split_data(const char *path){
	FILE *raw_filp = fopen(path, "r");
	if(raw_filp == NULL){
		perror("solve_topk fopen raw input");
		return errno;
	}

	struct timeval t1, t2;
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
//#define BUFF_SPLIT
#ifdef BUFF_SPLIT
	int *shard_off = new int[SHARD_SIZE];
	memset(shard_off, 0, sizeof(int)*SHARD_SIZE);
	char **shard_buf = new char*[SHARD_SIZE];
	for(int i = 0; i < SHARD_SIZE; i++){
		shard_buf[i] = new char[MIN_FILE_SIZE/SHARD_SIZE*2];
		memset(shard_buf[i], 0, MIN_FILE_SIZE/SHARD_SIZE*2);
		//for(int j = 0; j < MIN_FILE_SIZE/SHARD_SIZE*2; j++)
			//shard_buf[i][j] = new char[MAX_URL_LEN];
	}


	gettimeofday(&t1, NULL);
	while(fscanf(raw_filp, "%s", buf) != EOF){
		int shard = calcu_hash(buf) % SHARD_SIZE;
		sprintf(shard_buf[shard] + shard_off[shard], "%s\n", buf);
		shard_off[shard] += strlen(buf) + 1;
		//fprintf(sub_filp[shard], "%s\n", buf);
	}
	for(int i = 0; i < SHARD_SIZE; i++){
		int shard = i;
		fprintf(sub_filp[shard], "%s", shard_buf[i]);
	}
#else
	gettimeofday(&t1, NULL);
	while(fscanf(raw_filp, "%s", buf) != EOF){
		int shard = calcu_hash(buf) % SHARD_SIZE;
		fprintf(sub_filp[shard], "%s\n", buf);
	}

#endif

	for(int i = 0; i < SHARD_SIZE; i++)
		fclose(sub_filp[i]);
	fclose(raw_filp);
	gettimeofday(&t2, NULL);
	double read_cost = TIME(t1, t2);
	printf("%s: read raw_input %.2fs\n", __func__, read_cost);
	delete []sub_filp;
	delete []sub_path;
	delete []buf;
	//printf("%s: hash_cost %.2fs\n", __func__, hash_cost);
	return 0;
}

static int read_split(const char *path){
	FILE **sub_filp = new FILE*[SHARD_SIZE];
	char *sub_path = new char[1024];
	char *buf = new char[MAX_URL_LEN];
	double read_cost = 0;
	struct timeval t1, t2;
	gettimeofday(&t1, NULL);
	ull x = 0;
	for(int i = 0; i < SHARD_SIZE; i++){
		sprintf(sub_path, "%s-sub-%d", path, i);
		sub_filp[i] = fopen(sub_path, "r");
		assert(sub_filp[i]);
		while(fscanf(sub_filp[i], "%s", buf) != EOF){
		}
		fclose(sub_filp[i]);
	}
	gettimeofday(&t2, NULL);
	read_cost = TIME(t1, t2);
	printf("read_split: %.2f, %llu\n", read_cost, x);
	
	delete []sub_filp;
	delete []sub_path;
	delete []buf;
}


int main(int argc, char *argv[]){
	char path[256];
	int scale = 1;
	sprintf(path, "/home/sirius/repos/test/pingcap/input/url%dG.in", scale);
	struct timeval t1, t2;
	gettimeofday(&t1, NULL);
	//read_one_file(path);
	//fscanf_one_file(path);
	split_data(path);
	//read_split(path);
	gettimeofday(&t2, NULL);
	printf("total cost: %.2f\n", TIME(t1, t2));
	size_t rss = getPeakRSS();
	printf("memory used max: %.2fMB\n", 1.0*rss/(1024*1024) );
	return 0;
}
