
#include "pitch_shift.h"
#include <alsa/asoundlib.h>
#include <unistd.h>
snd_pcm_t *pcm_c;			//capture device
snd_pcm_t *pcm_p;			//playback device

char *name = "default";			//name of device

snd_pcm_hw_params_t *params_c, *params_p;

void print_usage(char *name);

int
main(int argc, char *argv[])
{
	int res, size;
	float *ibuff, *obuff;
	float shift;
	
	if (argc < 2) {
		print_usage(argv[0]);
		return 1;
	}
	shift = atof(argv[1]);
	
	if (shift < 0.5 || shift > 2.0)
		shift = 0.8;
	
	snd_pcm_uframes_t frames;
	
	frames = 1024;
	
	size = frames * 4;
	ibuff = malloc(size);
	obuff = malloc(size);
	
	res = snd_pcm_open(&pcm_c, name, SND_PCM_STREAM_CAPTURE, 0);
	if (res != 0) {
		printf("open for capture err\n");
		exit(1);
	}
	
	res = snd_pcm_open(&pcm_p, name, SND_PCM_STREAM_PLAYBACK, 0);
	if (res != 0) {
		printf("open for playback err\n");
		exit(1);
	}

	res = snd_pcm_set_params(pcm_p, SND_PCM_FORMAT_FLOAT_LE,
				     SND_PCM_ACCESS_RW_INTERLEAVED, 1, 44100, 1, 100000);
	
	if (res != 0) {
		fprintf(stderr, "unable to set params(: %s\n", snd_strerror(res));
		exit(1);
	}
	
	res = snd_pcm_set_params(pcm_c, SND_PCM_FORMAT_FLOAT_LE,
				     SND_PCM_ACCESS_RW_INTERLEAVED, 1, 44100, 1, 0);
	if (res != 0) {
		fprintf(stderr, "unable to set params(: %s\n", snd_strerror(res));
		exit(1);
	}
	
	snd_pcm_prepare(pcm_c);
	snd_pcm_prepare(pcm_p);

	while (1) {
		
		res = snd_pcm_readi(pcm_c, ibuff, frames);
		if (res == -EPIPE) {
			// EPIPE means overrun
			fprintf(stderr, "overrun occurred\n");
			snd_pcm_prepare(pcm_c);
		} else if (res < 0)
			fprintf(stderr, "error from read: %s\n", snd_strerror(res));
		else if (res != (int)frames)
			fprintf(stderr, "short read, read %d frames\n", res);

		pitch_shift(shift, frames, 512, 1, 44100, ibuff, obuff);
		
		res = snd_pcm_writei(pcm_p, obuff, frames);		//write(1, obuff, frames * sizeof(float));

		if (res == -EPIPE) {
			// EPIPE means underrun
			fprintf(stderr, "underrun occurred\n");
			snd_pcm_prepare(pcm_p);
		} else if (res < 0)
			fprintf(stderr, "error from writei: %s\n", snd_strerror(res));
		else if (res != (int)frames)
			fprintf(stderr, "short write, write %d frames\n", res);

	}

	snd_pcm_drain(pcm_c);
	snd_pcm_drain(pcm_p);

	snd_pcm_close(pcm_c);
	snd_pcm_close(pcm_p);
	
	free(ibuff);
	free(obuff);

	return 0;
}

void
print_usage(char *pname)
{
	printf("USAGE:\n %s shift(range 0.5 - 2)\n", pname);
}
