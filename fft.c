#include <math.h>
#include <stdio.h>

static int log2_int(int n);

/*
 * simply FAST FURIER TRANSLATION
 * Idea from Smith Scientist and
 * Engineers guide to DSP
 * second edition
 */

static inline int
main_fft(float *rex, float *imx, int n)
{
	int i, ip, j, k, m;
	int le, le2;
	int nd2;
	float si, sr, ti, tr, ui, ur;
	
	nd2 = n / 2;
	
	if (n <= 0 || rex == NULL || imx == NULL)
		return -1;
	/* Bit rev sorting
	 * Idea from Smith Scientist and
	 * Engineers guide to DSP
	 * second edition
	*/
	j = nd2;
	for (i = 1; i < n - 1; i++) {
		if ( i < j) {
			tr = rex[j];
			ti = imx[j];
			rex[j] = rex[i];
			imx[j] = imx[i];
			rex[i] = tr;
			imx[i] = ti;
		}
		k = nd2;
		while (k <= j) {
			j -= k;
			k >>= 1;
		}
		j = j + k;
	}
	
	m = log2_int(n);
	le = 1;
	
	for (k = 1; k <= m; k++) {					//Each stage
		le2 = le;
		le <<= 1;
		
		ur = 1;
		ui = 0;
		
		sr = cos(M_PI / le2);
		si = -sin(M_PI / le2);
		
		for (j = 1; j <= le2; j++) {				//Each SUB DFT
			for (i = j - 1; i < n; i += le) {		//Each butterfly
				ip = i + le2;				//Butterfly
				tr = rex[ip] * ur - imx[ip] * ui;
				ti = rex[ip] * ui + imx[ip] * ur;
				
				rex[ip] = rex[i] - tr;
				imx[ip] = imx[i] - ti;
				
				rex[i] = rex[i] + tr;
				imx[i] = imx[i] + ti;
			}
			tr = ur;
			ur  = tr * sr - ui * si;
			ui = tr * si + ui * sr;
		}
	}
	
	return 0;
}


int
fft(float *rex, float *imx, int n)
{
	main_fft(rex, imx, n);
	
	return 0;
}


int
fft_rev(float *rex, float *imx, int n)
{
	int i;
	
	for (i = 0; i < n; i++)
		imx[i] = -imx[i];

	main_fft(rex, imx, n);

	return 0;
}


/*
 * return log2(n)
 * rounding tolower
 * n must be > 1
*/
static int
log2_int(int n)
{
	int i;
	i = 0;
	while (n > 1) {
		n >>= 1;
		i++;
	}
	
	return i;
}
