//by Norbert Niderla, 2021

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "distribution.h"

#define SYMBOLS (256)
#define ENABLE_DEBUG (0)

uint8_t setFrequencies(uint8_t *input, int size, uint16_t *freqs) {
	int cnt = 0;
	for (int i = 0; i < size; i++)
		if (input[i] == 0)
			cnt++;
	for (int i = 1; i < SYMBOLS; i++)
		freqs[i] = 1;

	float dens = ((float) cnt / (float) size);
	dens = round(DENS_RANGE * dens) / DENS_RANGE;

	if (dens == 1)
		freqs[0] = 10000;
	else if (dens == 0)
		freqs[0] = 1;
	else
		freqs[0] = (int) (-dens * SYMBOLS / (dens - 1));

	return (uint8_t) (dens * DENS_RANGE);
}

void setFrequencies_Dens(int size, uint16_t *freqs, uint8_t dens_int) {
	for (int i = 1; i < size; i++)
		freqs[i] = 1;
	float dens = ((float) dens_int) / DENS_RANGE;
	if (dens == 1)
		freqs[0] = 10000;
	else if (dens == 0)
		freqs[0] = 1;
	else
		freqs[0] = (int) (-dens * SYMBOLS / (dens - 1));
}

void setOccurrences(int *input, int size, int L, uint16_t *occ) {
	int cnt = 0;
	for (int i = 0; i < size; i++)
		if (input[i] == 0)
			cnt++;
	float dens = ((float) cnt / (float) size);
	if (dens == 1)
		occ[0] = L - SYMBOLS + 1;
	else if (dens == 0)
		occ[0] = 1;
	else
		occ[0] = (int) round(dens * (float) L);
#if ENABLE_DEBUG
	printf("%d\n",(int)(dens*100));
#endif

	for (int i = 1; i < SYMBOLS; i++)
		occ[i] = 1;

	int missing_bits = L - occ[0] - SYMBOLS + 1;
	int idx = 0;

	if (missing_bits > 0) {
		while (missing_bits != 0) {
			occ[idx++]++;
			missing_bits--;
			if (idx == SYMBOLS)
				idx = 0;
		}
	} else if (missing_bits < 0) {
		occ[0] += missing_bits;
	}
}

void setOccurrences_char(unsigned char*input, int size, int L, uint16_t *occ) {
	int cnt = 0;
	for (int i = 0; i < size; i++)
		if (input[i] == 0)
			cnt++;
	float dens = ((float) cnt / (float) size);
	if (dens == 1)
		occ[0] = L - SYMBOLS + 1;
	else if (dens == 0)
		occ[0] = 1;
	else
		occ[0] = (int) round(dens * (float) L);
#if ENABLE_DEBUG
	printf("%d\n",(int)(dens*100));
#endif

	for (int i = 1; i < SYMBOLS; i++)
		occ[i] = 1;

	int missing_bits = L - occ[0] - SYMBOLS + 1;
	int idx = 0;

	if (missing_bits > 0) {
		while (missing_bits != 0) {
			occ[idx++]++;
			missing_bits--;
			if (idx == SYMBOLS)
				idx = 0;
		}
	} else if (missing_bits < 0) {
		occ[0] += missing_bits;
	}
}

int NormalPDF(double x, double sigma, double count_sum) {
	return (int) round(
			(1.0 / (sigma * 2.478)) * exp(-0.5 * (x / sigma) * (x / sigma))
					* count_sum);
}

void setOccurrencesNormal(int *input, int size, int L, uint16_t *occ) {

	double sd = 0;
	for (int k = 0; k < size; k++)
		sd += input[k] ^ 2;
	if (sd != 0)
		sd = sqrt((sd / (double) size));

	for (int k = 0; k < SYMBOLS; k++) {
		occ[k] = NormalPDF(k, sd, L);
		if (occ[k] == 0) {
			for (int l = k + 1; l < SYMBOLS; l++)
				occ[l] = 0;
			k = SYMBOLS;
		}
	}

	int sum = 0;
	for (int i = 0; i < SYMBOLS; i++)
		sum += occ[i];
	float coeff = (float) sum / (float) L;
	for (int i = 0; i < SYMBOLS; i++) {
		occ[i] = (int) round(((float) occ[i]) / coeff);
		if (occ[i] < 1)
			occ[i] = 1;
	}

	sum = 0;
	for (int i = 0; i < SYMBOLS; i++)
		sum += occ[i];

	int m_b = L - sum;
	if (m_b > 0) {
		int idx = 0;
		while (m_b != 0) {
			occ[idx++]++;
			m_b--;
			if (idx == SYMBOLS)
				idx = 0;
		}
	} else if (m_b < 0) {
		int idx = 0;
		while (m_b != 0) {
			if (occ[idx] > 1) {
				occ[idx]--;
				m_b++;
			}
			idx++;
			if (idx == SYMBOLS)
				idx = 0;
		}
	}
}

void setOccurrencesNormal_char(unsigned char*input, int size, int L, uint16_t *occ) {

	double sd = 0;
	for (int k = 0; k < size; k++)
		sd += input[k] ^ 2;
	if (sd != 0)
		sd = sqrt((sd / (double) size));

	for (int k = 0; k < SYMBOLS; k++) {
		occ[k] = NormalPDF(k, sd, L);
		if (occ[k] == 0) {
			for (int l = k + 1; l < SYMBOLS; l++)
				occ[l] = 0;
			k = SYMBOLS;
		}
	}

	int sum = 0;
	for (int i = 0; i < SYMBOLS; i++)
		sum += occ[i];
	float coeff = (float) sum / (float) L;
	for (int i = 0; i < SYMBOLS; i++) {
		occ[i] = (int) round(((float) occ[i]) / coeff);
		if (occ[i] < 1)
			occ[i] = 1;
	}

	sum = 0;
	for (int i = 0; i < SYMBOLS; i++)
		sum += occ[i];

	int m_b = L - sum;
	if (m_b > 0) {
		int idx = 0;
		while (m_b != 0) {
			occ[idx++]++;
			m_b--;
			if (idx == SYMBOLS)
				idx = 0;
		}
	} else if (m_b < 0) {
		int idx = 0;
		while (m_b != 0) {
			if (occ[idx] > 1) {
				occ[idx]--;
				m_b++;
			}
			idx++;
			if (idx == SYMBOLS)
				idx = 0;
		}
	}
}



