#include <ctime>

#include "common.h"

void gen_data(const char *path, const ull size, const int min_url_len=MIN_URL_LEN, const int max_url_len=MAX_URL_LEN){
	//assert(size >= MIN_FILE_SIZE);
	FILE *filp = fopen(path, "w+");
	if(!filp) {
		perror("error happens when gen_data");
		return;
	}
	//srand(time(NULL));
	//int rows = size / max_url_len + rand() % (size / min_url_len - size / max_url_len);
	int rows = size / ((max_url_len + min_url_len)/2);
	printf("%s: row %d\n", __func__, rows);
	char *buf = new char[max_url_len];
	ull file_size = 0;

	int start_rand = 0;;
#ifdef DEBUG_RECALL
	start_rand = 1000;
#endif


	for(int i = 0; i < rows; i++){
		int len;
		if(i >= start_rand){
			len = min_url_len + rand() % (max_url_len - min_url_len);
			for(int j = 0; j < len; j++)
				buf[j] = rand() % 26 + 'a';
		}else{
			len = 1;
			for(int j = 0; j < len; j++)
				buf[j] = rand() % 3 + 'a';
		}
		buf[len] = '\0';

		fprintf(filp, "%s\n", buf);
		file_size += len + 1;
	}
	delete []buf;
	fclose(filp);
	printf("data file total %llu bytes\n", file_size);
	printf("gen_data finish-----------\n");
}

/*
int main(){
	//printf("%d\n", RAND_MAX);
	gen_data("/home/sirius/repos/test/pingcap/input/url10G.in", 1ull*10*MIN_FILE_SIZE);
	//gen_data("./input/tmp.in", 1024);
	return 0;
}
*/
