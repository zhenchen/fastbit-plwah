//Analyze the distribution of all types of codewords in the index files of Plwah.
#include<stdint.h>
#include<stdio.h>
#include<malloc.h>
#define TYPES 8
using namespace std;

static uint32_t ONE_F_ONE = 0xf0000000;
static uint32_t ONE_F_ZERO = 0xe0000000;
static uint32_t ONE_F = 0xc0000000;
static uint32_t ZERO_F_ONE = 0xb0000000;
static uint32_t ZERO_F_ZERO = 0xa0000000;
static uint32_t ZERO_F = 0x80000000;
static uint32_t LITERAL = 0x00000001;
static uint32_t ZERO = 0x00000000;
static char names[TYPES][16] ={"ONE_F_ONE", "ONE_F_ZERO", "ONE_F", "ZERO_F_ONE", "ZERO_F_ZERO", "ZERO_F", "LITERAL", "ZERO"};

int main(void) {
	char filename[256];
	FILE * fp;
	unsigned results[TYPES];
	unsigned num[TYPES];
	int i, j, cnt;
	uint32_t *tmp;
	uint32_t check;
	printf("Input the filename: ");
	scanf("%s", filename);
	fp = fopen(filename, "rb");
	for (i=0; i<TYPES; ++i) results[i]=0;

	//read header
	char head[8];
	fread(head, sizeof(char), 8, fp);
	
	//read dimensions
	uint32_t dim[4];
	fread(dim, sizeof(uint32_t), 4, fp);
	
	//read values
	uint32_t begin, end;
    begin = 8*((3*sizeof(uint32_t) + 15) / 8);
    end = begin + dim[2] * sizeof(double);
	double *vals = (double*)malloc(sizeof(double)*dim[2]);
	fread(vals, sizeof(double), dim[2], fp);

	//read offset
	uint32_t * offset=(uint32_t*)malloc(sizeof(uint32_t)*(dim[1]+1));
	fread(offset, sizeof(uint32_t), dim[1]+1, fp);
	printf("The nrows indicated by the bitmap is: %u\n", dim[0]);
	printf("The nobs indicated by the bitmap is: %u\n", dim[1]);

	//read bitmap
	//if (OFFSET_LENGTH_MODE == 4 || OFFSET_LENGTH_MODE == 8)
	if (head[6] == 4 || head[6] == 8) {
		for(i=0; i<dim[1]; ++i){
			tmp = (uint32_t*)malloc(head[6]*(offset[i+1]-offset[i]));
			for (j=0; j<TYPES; ++j) num[j]=0;
			if (tmp != NULL) {
				cnt = (offset[i+1]-offset[i])*head[6]/sizeof(uint32_t);
				fseek(fp, offset[i], SEEK_SET);
				fread(tmp, sizeof(uint32_t), cnt,fp);

				for (j=0; j<cnt-2; ++j) {
					check = (tmp[j] & 0xf0000000);
					if (check >= ONE_F_ONE) ++num[0];
					else if (check >= ONE_F_ZERO) ++num[1];
					else if (check >= ONE_F) ++num[2];
					else if (check >= ZERO_F_ONE) ++num[3];
					else if (check >= ZERO_F_ZERO) ++num[4];
					else if (check >= ZERO_F) ++num[5];
					else if (check >= LITERAL) ++num[6];
					else if (check >= ZERO) ++num[7];
				}
				free(tmp);
				for (j=0; j<TYPES; ++j)
					results[j] += num[j];
		/*		if (num[0]>0 || num[1]>0) {
					printf("offset %u\n", offset[i]);
					printf("%u  %u\n", num[0], num[1]);
					break;
				}
				printf("num %lf\n",vals[i]);
				for (j=0; j<TYPES; ++j)
					printf("TYPE %d: %u    ", j, num[j]);
				printf("\n");*/
			}
		}
	}
	else printf("OFFSET_LENGTH_MODE is wrong\n");
	fclose(fp);
	for (j=0; j<TYPES; ++j) printf("%s: %u\n", names[j], results[j]);
	return 0;
}
