#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#define SYMBOLS 256

static float dens = 0;
static int n_calc = 0;

void resetDensCounter(void){
	dens = 0;
	n_calc = 0;
}

void setFrequencies(unsigned char* input, int size, int* freqs){
	int cnt = 0;
	for(int i = 0; i<size; i++) if(input[i]==0) cnt++;
	dens = dens + ((float)cnt/(float)size-dens)/(++n_calc);
	if(dens==1) freqs[0] = 10000;
	else if(dens==0) freqs[0] = 1;
	else freqs[0] = (int)(-dens*SYMBOLS/(dens-1));
}

static int calcMean(unsigned char* input, int size){
	int sum = 0;
	for(int i = 0; i<size; i++) sum += (int)input[i];
	return (int)sum/size;
}

int setOccurrences(int* input, int size, int L, uint16_t* occ){
	int cnt = 0;
	for(int i = 0; i<size; i++) if(input[i]==0) cnt++;
	dens = dens + ((float)cnt/(float)size-dens)/(++n_calc);
	int param = (int)round(dens*256);
	dens = (float)(param)/256;
	if(dens==1) occ[0] = 10000;
	else if(dens==0) occ[0] = 1;
	else occ[0] = (int)(-dens*(SYMBOLS)/(dens-1));

	int sum = 0;
	for(int i = 0; i<SYMBOLS; i++) sum += occ[i];
	float coeff = (float)sum/(float)L;
	for(int i = 0; i<SYMBOLS; i++){
		occ[i] = (int)round(((float)occ[i])/coeff);
		if(occ[i]<1) occ[i] = 1;
	}

	sum = 0;
	for(int i = 0; i<SYMBOLS; i++) sum += occ[i];

	int m_b  = L-sum;
	if(m_b>0){
		int idx = 0;
		while(m_b!=0){
			occ[idx++]++;
			m_b--;
			if(idx==256) idx=0;
		}
	}
	else if(m_b<0){
		bool lin_flag = true;
		for(int l = 1; l<256; l++) if(occ[l]!=1) lin_flag = false;
		if(lin_flag == true) occ[0] = occ[0]+m_b;
		else{
			int idx = 255;
			while(m_b!=0){
				if(occ[idx]>1){
					occ[idx]--;
					m_b++;
				}
				idx--;
				if(idx==-1) idx=255;
			}
		}
	}

	return param;
}

int NormalPDF(double x, double sigma, double count_sum){
    return (int)round((1.0/(sigma*2.478))*exp(-0.5*(x/sigma)*(x/sigma))*count_sum);
}

int setOccurrencesNormal(int* input, int size, int L, uint16_t* occ){

	double sd = 0;
	for(int k = 0; k<size; k++)sd += input[k]^2;
	if(sd != 0)sd = sqrt((sd/(double)size));

	for(int k = 0; k<size; k++) occ[k] = NormalPDF(k, sd, L);

	int sum = 0;
	for(int i = 0; i<SYMBOLS; i++) sum += occ[i];
	float coeff = (float)sum/(float)L;
	for(int i = 0; i<SYMBOLS; i++){
		occ[i] = (int)round(((float)occ[i])/coeff);
		if(occ[i]<1) occ[i] = 1;
	}

	sum = 0;
	for(int i = 0; i<SYMBOLS; i++) sum += occ[i];

	int m_b  = L-sum;
	if(m_b>0){
		int idx = 0;
		while(m_b!=0){
			occ[idx++]++;
			m_b--;
			if(idx==256) idx=0;
		}
	}
	else if(m_b<0){
		int idx = 0;
		while(m_b!=0){
			if(occ[idx]>1){
				occ[idx]--;
				m_b++;
			}
			idx++;
			if(idx==256) idx=0;
		}
	}

	return (int)sd;
}



