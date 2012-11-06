#ifndef PINTCH_SHIFT_H_
#define PINTCH_SHIFT_H_

#include <math.h>

#include "fft.h"

void pitch_shift(float shift, int sz, int N, int osampl, float smpl_rate, float *input, float *output);

#endif
