#include <ctime>

#include "common.h"

void gen_data(const char *path, const ull size){
	//assert(size >= MIN_FILE_SIZE);
	FILE *filp = fopen(path, "w+");
	if(!filp) {
		perror("error happens when gen_data");
		return;
	}
	//srand(time(NULL));
	int rows = size / MAX_URL_LEN + rand() % (size / MIN_URL_LEN - size / MAX_URL_LEN);
	printf("%s: row %d\n", __func__, rows);
	char *buf = new char[MAX_URL_LEN];
	ull file_size = 0;
	for(int i = 0; i < rows; i++){
		int len = MIN_URL_LEN + rand() % (MAX_URL_LEN - MIN_URL_LEN);
		for(int j = 0; j < len; j++)
			buf[j] = rand() % 26 + 'a';
		buf[len] = '\0';
		fprintf(filp, "%s\n", buf);
		file_size += len + 1;
	}
	delete []buf;
	fclose(filp);
	printf("data file total %llu bytes\n", file_size);
	printf("-----------------------\n");
}

/*
int main(){
	//printf("%d\n", RAND_MAX);
	gen_data("/home/sirius/repos/test/pingcap/input/url10G.in", 1ull*10*MIN_FILE_SIZE);
	//gen_data("./input/tmp.in", 1024);
	return 0;
}
*/
