#include "pitch_shift.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

enum {
	TRUE = 1,
	FALSE = 0,
	MAX_BUF_SIZE = 8192,
};


void 
pitch_shift(float shift, int sz, int N, int osampl, float smpl_rate, float *input, float *output)
{
	float work_rex[MAX_BUF_SIZE];
	float work_imx[MAX_BUF_SIZE];
	
	float proc_mag[MAX_BUF_SIZE / 2];
	float proc_freq[MAX_BUF_SIZE / 2];
	float syn_mag[MAX_BUF_SIZE / 2];
	float syn_freq[MAX_BUF_SIZE / 2];
	static float sum_phase[MAX_BUF_SIZE / 2];
	static float last_phase[MAX_BUF_SIZE / 2];
	static float out_accum[2 * MAX_BUF_SIZE];
	static float in_fifo[MAX_BUF_SIZE];
	static float out_fifo[MAX_BUF_SIZE];
	
	static int init = TRUE, rov;
	
	double rex, imx, mag, phase, window;
	double expct, tmp, fpb;				
	
	int i, k, ind, N2, ppp, step_sz, lat;				// ppp is PI per phase
	
	//
	step_sz = N / osampl;
	expct = 2. * M_PI * step_sz / N;
	lat = N - step_sz;
	fpb = smpl_rate / N;
	N2 = N / 2;
	
	rov = lat;
	
	if (init == TRUE) {
		memset(last_phase, 0, MAX_BUF_SIZE / 2 * sizeof(float));
		memset(sum_phase, 0, MAX_BUF_SIZE / 2 * sizeof(float));
		memset(in_fifo, 0, MAX_BUF_SIZE * sizeof(float));	
		memset(out_fifo, 0, MAX_BUF_SIZE * sizeof(float));
		memset(out_accum, 0, 2 * MAX_BUF_SIZE * sizeof(float));
		init = FALSE;
	}
	window = 1;
	/* for every sample in window */
	for (k = 0; k < sz; k ++) {
		
		in_fifo[rov] = input[k];
		output[k] = out_fifo[rov - lat];
		rov++;
		
		if (rov >= N) {
			rov = lat;
			for (i = 0; i < N; i++) {
				//window = -0.5 * cos(2.0 * M_PI * i / N) + 0.5;
				work_rex[i] = window * in_fifo[i];
				work_imx[i] = 0;
			}

			fft(work_rex, work_imx, N);

			/* Processing step */
			for (i = 0; i < N2; i++) {

				rex = work_rex[i];
				imx = work_imx[i];

				mag = 2.0 * sqrt(rex * rex + imx * imx);
				phase = atan2(imx, rex);
				
				//phase difference
				tmp = phase;

				proc_mag[i] = mag;
				proc_freq[i] = tmp;
			}
			
			/* Pitch_shift */
			memset(syn_mag, 0, MAX_BUF_SIZE / 2);
			memset(syn_freq, 0, MAX_BUF_SIZE / 2);

			for (i  = 0; i < N2; i++) {
				ind = i * shift;
				if (ind < N2) {
					syn_mag[ind] += proc_mag[i];
					syn_freq[ind] = proc_freq[i];
				}
			}
			
			/* Syntesis step */
			for (i = 0; i < N2; i++) {
				mag = syn_mag[i];
				tmp = syn_freq[i];
				
				phase = tmp;
				
				work_rex[i] = mag * cos(phase);
				work_imx[i] = mag * sin(phase);
			}
			
			/* zero neg freqs */
			for (i = N2 + 2; i < 2 * N; i++) {
				work_rex[i] = 0.0;
				work_imx[i] = 0.0;
			}

			fft_rev(work_rex, work_imx, N);
			
			for (i = 0; i < N; i++) {
				//window = -0.5 * cos(2.0 * M_PI * i / N) + 0.5;
				out_accum[i] += 2.0 * window * work_rex[i] / (N2 * osampl);
			}
			
			for (i = 0; i < step_sz; i++)
				out_fifo[i] = out_accum[i];
			
			memmove(out_accum, out_accum + step_sz, N * sizeof(*out_accum));
			
			for (i = 0; i < lat; i++)
				in_fifo[i] = in_fifo[i + step_sz];
		}
	}
}
