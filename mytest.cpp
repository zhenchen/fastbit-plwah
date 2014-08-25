#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<vector>
typedef unsigned uint32_t;

const uint32_t MAXBITS = 8*sizeof(uint32_t) - 1;
const uint32_t SECONDBIT = MAXBITS - 1;
const uint32_t ALLONES = ((1U << MAXBITS) - 1);
const uint32_t MAXCNT = ((1U << SECONDBIT) - 1);
const uint32_t MAXCNT_PLWAH = MAXCNT >> 1;
const uint32_t PIGGYBACK = (1U << (SECONDBIT - 1));
const uint32_t MAXCNT_PB = ((1U << (MAXBITS - 8)) - 1);
const uint32_t FILLBIT =(1U << SECONDBIT);
const uint32_t HEADER0 = (2U << SECONDBIT);
const uint32_t HEADER1 = (3U << SECONDBIT);
const uint32_t PIGGY_TO_FILL = ((3U << SECONDBIT) | MAXCNT_PB);
const uint32_t PIGGY_TO_LITERAL = (((1U << (MAXBITS - 3)) - 1U) & (~MAXCNT_PB));

using namespace std;

vector<uint32_t>& compress_plwah(vector<uint32_t>& m_vec);
vector<uint32_t>& decompress_plwah(vector<uint32_t>& m_vec, unsigned size);

int main() {
	FILE *in = fopen("end_wah.idx", "rb");
	FILE *out = fopen("end_wah_1.idx", "wb");
	FILE *tmp = fopen("end_plwah_1.idx", "wb");

	//read header
	char head[8];
	fread(head, sizeof(char), 8, in);
	
	//read dimensions
	uint32_t dim[4];
	fread(dim, sizeof(uint32_t), 4, in);

	//read value
    //uint32_t begin, end;
    //begin = 8*((3*sizeof(uint32_t) + 15) / 8);
    //end = begin + dim[2] * sizeof(double);
	double *vals = (double*)malloc(sizeof(double)*dim[2]);
	fread(vals, sizeof(double), dim[2], in);

	//read offset
	uint32_t *offset=(uint32_t*)malloc(sizeof(uint32_t)*(dim[1]+1));
	fread(offset, sizeof(uint32_t), dim[1]+1, in);
	printf("The nrows indicated by the bitmap is: %u\n", dim[0]);
	printf("The nobs indicated by the bitmap is: %u\n", dim[1]);
	//_read(fdes,offset,sizeof(uint32_t)*(len[1]+1));

	fwrite(head, sizeof(char), 8, out);
	fwrite(dim, sizeof(uint32_t), 4, out);
	fwrite(vals,sizeof(double), dim[2], out);
	fwrite(offset, sizeof(uint32_t), dim[1]+1, out);

	fwrite(head, sizeof(char), 8, tmp);
	fwrite(dim, sizeof(uint32_t), 4, tmp);
	fwrite(vals,sizeof(double), dim[2], tmp);
	fwrite(offset, sizeof(uint32_t), dim[1]+1, tmp);

	//read bitmap
	unsigned i, j, num;
	uint32_t *vec;
	for(i=0;i<dim[1];++i) { 
		vec = (uint32_t*)malloc((offset[i+1]-offset[i])*sizeof(char));
		num = (offset[i+1]-offset[i])/sizeof(uint32_t);
		fread(vec, sizeof(uint32_t), num, in);
		vector<uint32_t>m_vec(vec, vec+num);
		compress_plwah(m_vec);
		for (j=0; j<m_vec.size(); ++j)
			vec[j]= m_vec[j];
		fwrite(vec, sizeof(uint32_t), m_vec.size(), tmp);
		decompress_plwah(m_vec, num);
		for (j=0; j<num; ++j)
			vec[j] = m_vec[j];
		fwrite(vec, sizeof(uint32_t), num, out);
		free(vec);
	}

	fclose(in);
	fclose(out);
	fclose(tmp);
}

vector<uint32_t>& compress_plwah(vector<uint32_t>& m_vec) {
    struct xrun {
	bool   isMatrix;
	bool   isPiggyBack;
	uint32_t dirty;
	vector<uint32_t>::iterator it;

	xrun() : isMatrix(false), isPiggyBack(false), dirty(0) {};
	void decode() {
	    unsigned cnt, pos;
		uint32_t tmp;
		isMatrix = ((*it > ALLONES) && ((*it & MAXCNT) <= MAXCNT_PB) && ((*it & PIGGYBACK) == 0));
		isPiggyBack = (!(*it > ALLONES));
		if (isPiggyBack) {
		    pos = 0;
		    tmp = *it;
		    while (pos<MAXBITS){
		        if ((tmp & 1U) == 0) break;
			    else {
		            tmp = tmp >> 1;
				    ++ pos;
			    }
		    }
			tmp = tmp >> 1;
			cnt = pos + 1;
			while (cnt<MAXBITS){
				if ((tmp & 1U) == 0) break;
				else {
					tmp = tmp >> 1;
					++ cnt;
				}
			}
			if (cnt>=MAXBITS && pos<MAXBITS){
				dirty = (PIGGYBACK | (pos << (MAXBITS-8)));
			}
			else{
			pos = 0;
			tmp = *it;
			while (pos<MAXBITS){
				if ((tmp & 1U) == 1) break;
				else {
					tmp = tmp >> 1;
					++ pos;
				}
			}
			tmp = tmp >> 1;
			cnt = pos + 1;
			while (cnt<MAXBITS){
				if ((tmp & 1U) == 1) break;
				else {
					tmp = tmp >> 1;
					++ cnt;
				}
			}
			if (cnt>=MAXBITS && pos<MAXBITS){
				dirty = (PIGGYBACK | (pos << (MAXBITS-8)) | (PIGGYBACK >>1 ));
			}
			else isPiggyBack = false;
			}
		}
	}
    };
    xrun last;	// point to the last code word in m_vec that might be modified
                // NOTE: last.nWords is not used by this function
    xrun current;// point to the current code to be examined

    current.it = m_vec.begin();
    last.it = m_vec.begin();
	last.decode();
    for (++ current.it; current.it < m_vec.end(); ++ current.it) {
		current.decode();
		// transform the solated full-zero words into 0-fills
		if (*last.it == 0x00000000) {
			*last.it = 0x80000001;
			last.isMatrix = true;
		}
		//merge
		if (last.isMatrix && current.isPiggyBack) {
			*(last.it) = ((*(last.it)) | (current.dirty));
			++ current.it;
			if (current.it >= m_vec.end()) break;
			else current.decode();
		}
		++ last.it;
		*(last.it) = *(current.it);
		last.isMatrix = current.isMatrix;
		last.isPiggyBack = current.isPiggyBack;
		last.dirty = current.dirty;
	}
	++ last.it;
    if (last.it < m_vec.end()) { // reduce the size of m_vec
	m_vec.erase(last.it, m_vec.end());
    }
	return (m_vec);
};

vector<uint32_t>& decompress_plwah(vector<uint32_t>& m_vec, unsigned size) {
	struct xrun {
		bool isPiggyBack;
		uint32_t Literal;
		uint32_t Fill;
		vector<uint32_t>::iterator it;

		xrun() : isPiggyBack(false), Literal(0U), Fill(0U) {};
		void decode() {
			isPiggyBack = (((*it >> (MAXBITS - 2)) | 2U) == 7U);
			if (isPiggyBack) {
				Fill = (*it & PIGGY_TO_FILL);
				Literal = (1U << ((*it & PIGGY_TO_LITERAL) >> (MAXBITS - 8)));
				if ((((*it >> (MAXBITS-3))) & 1U) != 1U) Literal = ~Literal;
				Literal = (Literal & ((1U << MAXBITS) - 1U ));
			}
		}
	};

	xrun current;// point to the current code to be examined
	xrun currentTmp;
	//initialize new bitvector. At last m_vec would be replaced by tmp_vec.
	
	uint32_t wahLength = size;  // read length of wah.
//	int cpxLength = m_vec.size();
	
//	std::cout<<"cpxLength"<<cpxLength<<std::endl;
//	std::cout<<"m_vec.size"<<m_vec.size()<<std::endl;
	
	vector<uint32_t> tmp_vec(wahLength,0);
	currentTmp.it = tmp_vec.begin();
	current.it = m_vec.begin();
	current.decode();

	for (; current.it <m_vec.end(); ++ current.it)
	{
		current.decode();
		if(!current.isPiggyBack)
		{
			*currentTmp.it = *current.it;
			//transform the 0x80000001 into a leteral
			if (*currentTmp.it ==0x80000001) *currentTmp.it = 0x00000000;
			
			currentTmp.it++;
		}
		else
		{
			*currentTmp.it = current.Fill;
			//transform the 0x80000001 into a leteral
			if (*currentTmp.it ==0x80000001) *currentTmp.it = 0x00000000;
			
			currentTmp.it++;
			*currentTmp.it = current.Literal;
			
			currentTmp.it++;
		}
//		std::cout<<std::hex<<*(m_vec.begin())<<std::endl;	
	}
	m_vec.swap(tmp_vec);//exanchge the two arrays.
	return m_vec;
};
