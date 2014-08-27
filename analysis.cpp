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

char StorageType[][16] = {
	"BINNING",	///!< ibis::bin.
	"RANGE",	///!< ibis::range.
	"MESA",	///!< ibis::interval.
	"AMBIT",	///!< ibis::ambit, range-range two level encoding on bins.
	"PALE",	///!< ibis::pale, equality-range encoding on bins.
	"PACK",	///!< ibis::pack, range-equality encoding on bins.
	"ZONE",	///!< ibis::zone, equality-equality encoding on bins.
	"RELIC",	///!< ibis::relic, the basic bitmap index.
	"ROSTER",	///!< ibis::roster, RID list.
	"SKIVE",	///!< ibis::skive, binary encoding with recoding of key values.
	"FADE",	///!< ibis::fade, multicomponent range encoding (unbinned).
	"SBIAD",	///!< ibis::sbiad, multicomponent interval encoding (unbinned).
	"SAPID",	///!< ibis::sapid, multicomponent equality encoding (unbinned).
	"EGALE",	///!< ibis::egale, multicomponent equality encoding on bins.
	"MOINS",	///!< ibis::moins, multicomponent range encoding on bins.
	"ENTRE",	///!< ibis::entre, multicomponent interval encoding on bins.
	"BAK",	///!< ibis::bak, reduced precision mapping, equality code.
	"BAK2",	///!< ibis::bak2, splits each BAK bin in two, one less than
		/// the mapped value, one greater and equal
		/// to the mapped value.
	"KEYWORDS",	///!< ibis::keywords, boolean term-document matrix.
	"MESH",	///!< not used.
	"BAND",	///!< not used.
	"DIREKTE",///!< ibis::direkte, hash value to bitmaps.
	"GENERIC",///!< not used.
	"BYLT",	///!< ibis::bylt, unbinned range-equality encoding.
	"FUZZ",	///!< ibis::fuzz, unbinned interval-equality encoding.
	"ZONA",	///!< ibis::zona, unbinned equality-equality encoding.
	"FUGE",	///!< ibis::fuge, binned interval-equality encoding.
	"SLICE",	///!< ibis::slice, bit-sliced index.
	"EXTERN"	///!< externally defined index
};

int main(void) {
	char filename[256];
	FILE * fp;
	unsigned results[TYPES];
	unsigned num[TYPES];
	unsigned long i, j, cnt;
	uint32_t *tmp;
	uint32_t check;
	uint32_t START = 1;
	
	printf("Input the filename: ");
	scanf("%s", filename);
	if (filename[0]=='w') START = 0;
	fp = fopen(filename, "rb");
	for (i=0; i<TYPES; ++i) results[i]=0;

	//read header
	char head[8];
	fread(head, sizeof(char), 8, fp);
	printf("The bitmap type: %u %s\n", head[5], StorageType[(unsigned int)head[5]]);
	printf("The offset length: %u\n", head[6]);
	
	//read the first two dimensions
	uint32_t dim[4];
	fread(dim, sizeof(uint32_t), 2, fp);
	printf("The nrows: %u\n", dim[0]);
	printf("The nobs: %u\n", dim[1]);
	
	switch (head[5]) {
	case 7:
	{
		//read the rest dimensions
		fread(dim+2, sizeof(uint32_t), 2, fp);
		//read values
		uint32_t begin, end;
		begin = 8*((3*sizeof(uint32_t) + 15) / 8);
		end = begin + dim[2] * sizeof(double);
		double *vals = (double*)malloc(sizeof(double)*dim[2]);
		fread(vals, sizeof(double), dim[2], fp);

		if (head[6] == 4) {
			//read offset
			uint32_t * offset=(uint32_t*)malloc(sizeof(uint32_t)*(dim[1]+1));
			fread(offset, sizeof(uint32_t), dim[1]+1, fp);

			//read bitmap
			for(i=0; i<dim[1]; ++i){
				if (offset[i+1]>offset[i]) {
					tmp = (uint32_t*)malloc(offset[i+1]-offset[i]);
					for (j=0; j<TYPES; ++j) num[j]=0;
					cnt = (offset[i+1]-offset[i])/sizeof(uint32_t);
					fseek(fp, offset[i], SEEK_SET);
					fread(tmp, sizeof(uint32_t), cnt,fp);

					for (j=START; j<cnt-2; ++j) {
						check = tmp[j];
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
					for (j=0; j<TYPES; ++j) results[j] += num[j];
				}
			}
		}
		else if  (head[6] == 8) {
			//read offset
			unsigned long * offset=(unsigned long*)malloc(sizeof(unsigned long)*(dim[1]+1));
			fread(offset, sizeof(unsigned long), dim[1]+1, fp);

			//read bitmap
			for(i=0; i<dim[1]; ++i){
				if (offset[i+1]>offset[i]) {
					tmp = (uint32_t*)malloc(offset[i+1]-offset[i]);
					for (j=0; j<TYPES; ++j) num[j]=0;
					cnt = (offset[i+1]-offset[i])/sizeof(uint32_t);
					fseek(fp, offset[i], SEEK_SET);
					fread(tmp, sizeof(uint32_t), cnt,fp);

					for (j=START; j<cnt-2; ++j) {
						check = tmp[j];
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
					for (j=0; j<TYPES; ++j) results[j] += num[j];
				}
			}
		}
		else printf("OFFSET_LENGTH_MODE is wrong\n");
		break;
	}
	case 21:
	{
		if (head[6] == 4) {
			//read offset
			uint32_t * offset=(uint32_t*)malloc(sizeof(uint32_t)*(dim[1]+1));
			fread(offset, sizeof(uint32_t), dim[1]+1, fp);

			//read bitmap
			for(i=0; i<dim[1]; ++i){
				if (offset[i+1]>offset[i]) {
					tmp = (uint32_t*)malloc(offset[i+1]-offset[i]);
					for (j=0; j<TYPES; ++j) num[j]=0;
					cnt = (offset[i+1]-offset[i])/sizeof(uint32_t);
					fseek(fp, offset[i], SEEK_SET);
					fread(tmp, sizeof(uint32_t), cnt,fp);

					for (j=START; j<cnt-2; ++j) {
						check = tmp[j];
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
					for (j=0; j<TYPES; ++j) results[j] += num[j];
				}
			}
		}
		else if  (head[6] == 8) {
			//read offset
			unsigned long * offset=(unsigned long*)malloc(sizeof(unsigned long)*(dim[1]+1));
			fread(offset, sizeof(unsigned long), dim[1]+1, fp);

			//read bitmap
			for(i=0; i<dim[1]; ++i){
				if (offset[i+1]>offset[i]) {
					tmp = (uint32_t*)malloc(offset[i+1]-offset[i]);
					for (j=0; j<TYPES; ++j) num[j]=0;
					cnt = (offset[i+1]-offset[i])/sizeof(uint32_t);
					fseek(fp, offset[i], SEEK_SET);
					fread(tmp, sizeof(uint32_t), cnt,fp);

					for (j=START; j<cnt-2; ++j) {
						check = tmp[j];
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
					for (j=0; j<TYPES; ++j) results[j] += num[j];
				}
			}
		}
		else printf("OFFSET_LENGTH_MODE is wrong\n");
		break;
	}
	default:
	{
		printf("Cannot analyze\n");
		break;
	}
	}
	fclose(fp);
	for (j=0; j<TYPES; ++j) printf("%s: %u\n", names[j], results[j]);
	return 0;
}
