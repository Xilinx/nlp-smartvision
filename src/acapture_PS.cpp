#include <iostream>
#define ALSA_PCM_NEW_HW_PARAMS_API
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <locale.h>
#include <assert.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <endian.h>
#include <alsa/asoundlib.h>
#include "acapture_init.h"

#ifdef SND_CHMAP_API_VERSION
#define CONFIG_SUPPORT_CHMAP	1
#endif

#ifndef LLONG_MAX
#define LLONG_MAX    9223372036854775807LL
#endif

#ifndef le16toh
#include <asm/byteorder.h>
#define le16toh(x) __le16_to_cpu(x)
#define be16toh(x) __be16_to_cpu(x)
#define le32toh(x) __le32_to_cpu(x)
#define be32toh(x) __be32_to_cpu(x)
#endif

#define FORMAT_DEFAULT		-1
#define FORMAT_RAW		0

static snd_pcm_sframes_t (*readi_func)(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writei_func)(snd_pcm_t *handle, const void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*readn_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writen_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);

static char *command;
static snd_pcm_t *handle;

static struct {
	snd_pcm_format_t format;
	unsigned int channels;
	unsigned int rate;
} hwparams, rhwparams;

extern int window_size;
extern int ddr_buffer;	
extern short *InputData;
extern bool *Record_flag;

static int timelimit = 0;
static int sampleslimit = 0;
static int quiet_mode = 0;
static int file_type = FORMAT_DEFAULT;
static int open_mode = 0;
static snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
static int interleaved = 1;
static volatile sig_atomic_t in_aborting = 0;
static u_char *audiobuf = NULL;
static snd_pcm_uframes_t chunk_size = 0;
static unsigned period_time = 0;
static unsigned buffer_time = 0;
static snd_pcm_uframes_t period_frames = 341;
static snd_pcm_uframes_t buffer_frames = 1364;
static int avail_min = -1;
static int start_delay = 0;
static int stop_delay = 0;
static int monotonic = 0;
static int interactive = 0;
static int can_pause = 0;
static int fatal_errors = 0;
static int verbose = 0;
static size_t significant_bits_per_sample, bits_per_sample, bits_per_frame;
static size_t chunk_bytes;
static int test_position = 0;
static int test_coef = 8;
static int test_nowait = 0;
volatile static int recycle_capture_file = 0;
static long term_c_lflag = -1;
static int fd = -1;
static off64_t pbrec_count = LLONG_MAX;

#ifdef CONFIG_SUPPORT_CHMAP
static snd_pcm_chmap_t *channel_map = NULL; /* chmap to override */
static unsigned int *hw_map = NULL; /* chmap to follow */
#endif

/* needed prototypes */

static void done_stdin(void);
static void capture();
static void suspend(void);

/*
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define error(...)                                                   \
	do                                                               \
	{                                                                \
		fprintf(stderr, "%s: %s:%d: ", command, __func__, __LINE__); \
		fprintf(stderr, __VA_ARGS__);                                \
		putc('\n', stderr);                                          \
	} while (0)
#else
#define error(args...)                                               \
	do                                                               \
	{                                                                \
		fprintf(stderr, "%s: %s:%d: ", command, __func__, __LINE__); \
		fprintf(stderr, ##args);                                     \
		putc('\n', stderr);                                          \
	} while (0)
#endif
*/
/*
 *	Subroutine to clean up before exit.
 */
static void prg_exit(int code)
{
	done_stdin();
	if (handle)
		snd_pcm_close(handle);

	exit(code);
}

static void signal_handler(int sig)
{
	if (in_aborting)
		return;

	in_aborting = 1;
	if (verbose==2)
		putchar('\n');
	if (!quiet_mode)
		fprintf(stderr, _("Aborted by signal %s...\n"), strsignal(sig));
	if (handle)
		snd_pcm_abort(handle);
	if (sig == SIGABRT) {
		/* do not call snd_pcm_close() and abort immediately */
		handle = NULL;
		prg_exit(EXIT_FAILURE);
	}
	signal(sig, SIG_DFL);
}

/* call on SIGUSR1 signal. */
static void signal_handler_recycle (int sig)
{
	/* flag the capture loop to start a new output file */
	recycle_capture_file = 1;
}

#ifdef CONFIG_SUPPORT_CHMAP
static int setup_chmap(void)
{
	snd_pcm_chmap_t *chmap = channel_map;
	char mapped[hwparams.channels];
	snd_pcm_chmap_t *hw_chmap;
	unsigned int ch, i;
	int err;

	if (!chmap)
		return 0;

	if (chmap->channels != hwparams.channels) {
		printf("Channel numbers don't match between hw_params and channel map");
		return -1;
	}
	err = snd_pcm_set_chmap(handle, chmap);
	if (!err)
		return 0;

	hw_chmap = snd_pcm_get_chmap(handle);
	if (!hw_chmap) {
		fprintf(stderr, _("Warning: unable to get channel map\n"));
		return 0;
	}

	if (hw_chmap->channels == chmap->channels &&
	    !memcmp(hw_chmap, chmap, 4 * (chmap->channels + 1))) {
		/* maps are identical, so no need to convert */
		free(hw_chmap);
		return 0;
	}

	hw_map = calloc(hwparams.channels, sizeof(int));
	if (!hw_map) {
		printf("not enough memory");
		free(hw_chmap);
		return -1;
	}

	memset(mapped, 0, sizeof(mapped));
	for (ch = 0; ch < hw_chmap->channels; ch++) {
		if (chmap->pos[ch] == hw_chmap->pos[ch]) {
			mapped[ch] = 1;
			hw_map[ch] = ch;
			continue;
		}
		for (i = 0; i < hw_chmap->channels; i++) {
			if (!mapped[i] && chmap->pos[ch] == hw_chmap->pos[i]) {
				mapped[i] = 1;
				hw_map[ch] = i;
				break;
			}
		}
		if (i >= hw_chmap->channels) {
			char buf[256];
			printf("Channel %d doesn't match with hw_parmas", ch);
			snd_pcm_chmap_print(hw_chmap, sizeof(buf), buf);
			fprintf(stderr, "hardware chmap = %s\n", buf);
			free(hw_chmap);
			return -1;
		}
	}
	free(hw_chmap);
	return 0;
}
#else
#define setup_chmap()	0
#endif

static void set_params(void)
{
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_uframes_t buffer_size;
	int err;
	size_t n;
	unsigned int rate;
	snd_pcm_uframes_t start_threshold, stop_threshold;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);
	
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		printf("Broken configuration for this PCM: no configurations available");
		prg_exit(EXIT_FAILURE);
	}
	
	if (interleaved)
		err = snd_pcm_hw_params_set_access(handle, params,
						   SND_PCM_ACCESS_RW_INTERLEAVED);
	else
		err = snd_pcm_hw_params_set_access(handle, params,
						   SND_PCM_ACCESS_RW_NONINTERLEAVED);
	if (err < 0) {
		printf("Access type not available");
		prg_exit(EXIT_FAILURE);
	}
	
	err = snd_pcm_hw_params_set_format(handle, params, hwparams.format);
	if (err < 0) {
		printf("Sample format non available");
		prg_exit(EXIT_FAILURE);
	}
	
	err = snd_pcm_hw_params_set_channels(handle, params, hwparams.channels);
	if (err < 0) {
		printf("Channels count non available");
		prg_exit(EXIT_FAILURE);
	}

#if 0
	err = snd_pcm_hw_params_set_periods_min(handle, params, 2);
	assert(err >= 0);
#endif

	rate = hwparams.rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &hwparams.rate, 0);
	assert(err >= 0);
	if ((float)rate * 1.05 < hwparams.rate || (float)rate * 0.95 > hwparams.rate) {
		if (!quiet_mode) {
			char plugex[64];
			const char *pcmname = snd_pcm_name(handle);
			fprintf(stderr, _("Warning: rate is not accurate (requested = %iHz, got = %iHz)\n"), rate, hwparams.rate);
			if (! pcmname || strchr(snd_pcm_name(handle), ':'))
				*plugex = 0;
			else
				snprintf(plugex, sizeof(plugex), "(-Dplug:%s)",
					 snd_pcm_name(handle));
			fprintf(stderr, _("         please, try the plug plugin %s\n"),
				plugex);
		}
	}
	
	rate = hwparams.rate;
	if (buffer_time == 0 && buffer_frames == 0) {
		err = snd_pcm_hw_params_get_buffer_time_max(params,
							    &buffer_time, 0);
		assert(err >= 0);
		if (buffer_time > 500000)
			buffer_time = 500000;
	}
	
	if (period_time == 0 && period_frames == 0) {
		if (buffer_time > 0)
			period_time = buffer_time / 4;
		else
			period_frames = buffer_frames / 4;
	}
	if (period_time > 0)
		err = snd_pcm_hw_params_set_period_time_near(handle, params,
							     &period_time, 0);
	else
		err = snd_pcm_hw_params_set_period_size_near(handle, params,
							     &period_frames, 0);
	assert(err >= 0);
	
	if (buffer_time > 0) {
		err = snd_pcm_hw_params_set_buffer_time_near(handle, params,
							     &buffer_time, 0);
	} else {
		err = snd_pcm_hw_params_set_buffer_size_near(handle, params,
							     &buffer_frames);
	}
	assert(err >= 0);
	
	monotonic = snd_pcm_hw_params_is_monotonic(params);
	can_pause = snd_pcm_hw_params_can_pause(params);
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
		printf("Unable to install hw params:");
		prg_exit(EXIT_FAILURE);
	}
	
	snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
	snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
	if (chunk_size == buffer_size) {
		printf("Can't use period equal to buffer size (%lu == %lu)",
		      chunk_size, buffer_size);
		prg_exit(EXIT_FAILURE);
	}
	
	err = snd_pcm_sw_params_current(handle, swparams);
	if (err < 0) {
		printf("Unable to get current sw params.");
		prg_exit(EXIT_FAILURE);
	}
	if (avail_min < 0)
		n = chunk_size;
	else
		n = (double) rate * avail_min / 1000000;
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, n);

	/* round up to closest transfer boundary */
	n = buffer_size;
	if (start_delay <= 0) {
		start_threshold = n + (double) rate * start_delay / 1000000;
	} else
		start_threshold = (double) rate * start_delay / 1000000;
	if (start_threshold < 1)
		start_threshold = 1;
	if (start_threshold > n)
		start_threshold = n;
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, start_threshold);
	assert(err >= 0);
	if (stop_delay <= 0)
		stop_threshold = buffer_size + (double) rate * stop_delay / 1000000;
	else
		stop_threshold = (double) rate * stop_delay / 1000000;
	err = snd_pcm_sw_params_set_stop_threshold(handle, swparams, stop_threshold);
	assert(err >= 0);

	if (snd_pcm_sw_params(handle, swparams) < 0) {
		printf("unable to install sw params:");
		prg_exit(EXIT_FAILURE);
	}

	if (setup_chmap())
		prg_exit(EXIT_FAILURE);


	bits_per_sample = snd_pcm_format_physical_width(hwparams.format);
	significant_bits_per_sample = snd_pcm_format_width(hwparams.format);
	bits_per_frame = bits_per_sample * hwparams.channels;
	chunk_bytes = chunk_size * bits_per_frame / 8;
	
	//
	//cout << "chunk size" << chunk_size << endl;
	//cout << "chunk bytes" << chunk_bytes << endl;
	//
	audiobuf = realloc(audiobuf, chunk_bytes);
	if (audiobuf == NULL) {
		printf("not enough memory");
		prg_exit(EXIT_FAILURE);
	}

	buffer_frames = buffer_size;	/* for position test */
}

static void init_stdin(void)
{
	struct termios term;
	long flags;

	if (!interactive)
		return;
	if (!isatty(fileno(stdin))) {
		interactive = 0;
		return;
	}
	tcgetattr(fileno(stdin), &term);
	term_c_lflag = term.c_lflag;
	if (fd == fileno(stdin))
		return;
	flags = fcntl(fileno(stdin), F_GETFL);
	if (flags < 0 || fcntl(fileno(stdin), F_SETFL, flags|O_NONBLOCK) < 0)
		fprintf(stderr, _("stdin O_NONBLOCK flag setup failed\n"));
	term.c_lflag &= ~ICANON;
	tcsetattr(fileno(stdin), TCSANOW, &term);
}

static void done_stdin(void)
{
	struct termios term;

	if (!interactive)
		return;
	if (fd == fileno(stdin) || term_c_lflag == -1)
		return;
	tcgetattr(fileno(stdin), &term);
	term.c_lflag = term_c_lflag;
	tcsetattr(fileno(stdin), TCSANOW, &term);
}

static void do_pause(void)
{
	int err;
	unsigned char b;

	if (!can_pause) {
		fprintf(stderr, _("\rPAUSE command ignored (no hw support)\n"));
		return;
	}
	if (snd_pcm_state(handle) == SND_PCM_STATE_SUSPENDED)
		suspend();

	err = snd_pcm_pause(handle, 1);
	if (err < 0) {
		printf("pause push error: %s", snd_strerror(err));
		return;
	}
	while (1) {
		while (read(fileno(stdin), &b, 1) != 1);
		if (b == ' ' || b == '\r') {
			while (read(fileno(stdin), &b, 1) == 1);
			if (snd_pcm_state(handle) == SND_PCM_STATE_SUSPENDED)
				suspend();
			err = snd_pcm_pause(handle, 0);
			if (err < 0)
				printf("pause release error: %s", snd_strerror(err));
			return;
		}
	}
}

static void check_stdin(void)
{
	unsigned char b;

	if (!interactive)
		return;
	if (fd != fileno(stdin)) {
		while (read(fileno(stdin), &b, 1) == 1) {
			if (b == ' ' || b == '\r') {
				while (read(fileno(stdin), &b, 1) == 1);
				fprintf(stderr, _("\r=== PAUSE ===                                                            "));
				fflush(stderr);
			do_pause();
				fprintf(stderr, "                                                                          \r");
				fflush(stderr);
			}
		}
	}
}

#ifndef timersub
#define	timersub(a, b, result) \
do { \
	(result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
	(result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
	if ((result)->tv_usec < 0) { \
		--(result)->tv_sec; \
		(result)->tv_usec += 1000000; \
	} \
} while (0)
#endif

#ifndef timermsub
#define	timermsub(a, b, result) \
do { \
	(result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
	(result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
	if ((result)->tv_nsec < 0) { \
		--(result)->tv_sec; \
		(result)->tv_nsec += 1000000000L; \
	} \
} while (0)
#endif

/* I/O error handler */
static void xrun(void)
{
	snd_pcm_status_t *status;
	int res;

	snd_pcm_status_alloca(&status);
	if ((res = snd_pcm_status(handle, status))<0) {
		printf("status error: %s", snd_strerror(res));
		prg_exit(EXIT_FAILURE);
	}
	if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
		if (fatal_errors) {
			printf("fatal %s: %s",
					stream == SND_PCM_STREAM_PLAYBACK ? _("underrun") : _("overrun"),
					snd_strerror(res));
			prg_exit(EXIT_FAILURE);
		}
		if (monotonic) {
#ifdef HAVE_CLOCK_GETTIME
			struct timespec now, diff, tstamp;
			clock_gettime(CLOCK_MONOTONIC, &now);
			snd_pcm_status_get_trigger_htstamp(status, &tstamp);
			timermsub(&now, &tstamp, &diff);
			fprintf(stderr, _("%s!!! (at least %.3f ms long)\n"),
				stream == SND_PCM_STREAM_PLAYBACK ? _("underrun") : _("overrun"),
				diff.tv_sec * 1000 + diff.tv_nsec / 1000000.0);
#else
			fprintf(stderr, "%s !!!\n", _("underrun"));
#endif
		} else {
			struct timeval now, diff, tstamp;
			gettimeofday(&now, 0);
			snd_pcm_status_get_trigger_tstamp(status, &tstamp);
			timersub(&now, &tstamp, &diff);
			fprintf(stderr, _("%s!!! (at least %.3f ms long)\n"),
				stream == SND_PCM_STREAM_PLAYBACK ? _("underrun") : _("overrun"),
				diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
		}

		if ((res = snd_pcm_prepare(handle))<0) {
			printf("xrun: prepare error: %s", snd_strerror(res));
			prg_exit(EXIT_FAILURE);
		}
		return;		/* ok, data should be accepted again */
	} if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING) {

		if (stream == SND_PCM_STREAM_CAPTURE) {
			fprintf(stderr, _("capture stream format change? attempting recover...\n"));
			if ((res = snd_pcm_prepare(handle))<0) {
				printf("xrun(DRAINING): prepare error: %s", snd_strerror(res));
				prg_exit(EXIT_FAILURE);
			}
			return;
		}
	}

	printf("read/write error, state = %s", snd_pcm_state_name(snd_pcm_status_get_state(status)));
	prg_exit(EXIT_FAILURE);
}

/* I/O suspend handler */
static void suspend(void)
{
	int res;

	if (!quiet_mode) {
		fprintf(stderr, _("Suspended. Trying resume. ")); fflush(stderr);
	}
	while ((res = snd_pcm_resume(handle)) == -EAGAIN)
		sleep(1);	/* wait until suspend flag is released */
	if (res < 0) {
		if (!quiet_mode) {
			fprintf(stderr, _("Failed. Restarting stream. ")); fflush(stderr);
		}
		if ((res = snd_pcm_prepare(handle)) < 0) {
			printf("suspend: prepare error: %s", snd_strerror(res));
			prg_exit(EXIT_FAILURE);
		}
	}
	if (!quiet_mode)
		fprintf(stderr, _("Done.\n"));
}


static void do_test_position(void)
{
	static long counter = 0;
	static time_t tmr = -1;
	time_t now;
	static float availsum, delaysum, samples;
	static snd_pcm_sframes_t maxavail, maxdelay;
	static snd_pcm_sframes_t minavail, mindelay;
	static snd_pcm_sframes_t badavail = 0, baddelay = 0;
	snd_pcm_sframes_t outofrange;
	snd_pcm_sframes_t avail, delay;
	int err;

	err = snd_pcm_avail_delay(handle, &avail, &delay);
	if (err < 0)
		return;
	outofrange = (test_coef * (snd_pcm_sframes_t)buffer_frames) / 2;
	if (avail > outofrange || avail < -outofrange ||
	    delay > outofrange || delay < -outofrange) {
	  badavail = avail; baddelay = delay;
	  availsum = delaysum = samples = 0;
	  maxavail = maxdelay = 0;
	  minavail = mindelay = buffer_frames * 16;
	  fprintf(stderr, _("Suspicious buffer position (%li total): "
	  	"avail = %li, delay = %li, buffer = %li\n"),
	  	++counter, (long)avail, (long)delay, (long)buffer_frames);
	} else if (verbose) {
		time(&now);
		if (tmr == (time_t) -1) {
			tmr = now;
			availsum = delaysum = samples = 0;
			maxavail = maxdelay = 0;
			minavail = mindelay = buffer_frames * 16;
		}
		if (avail > maxavail)
			maxavail = avail;
		if (delay > maxdelay)
			maxdelay = delay;
		if (avail < minavail)
			minavail = avail;
		if (delay < mindelay)
			mindelay = delay;
		availsum += avail;
		delaysum += delay;
		samples++;
		if (avail != 0 && now != tmr) {
			fprintf(stderr, "BUFPOS: avg%li/%li "
				"min%li/%li max%li/%li (%li) (%li:%li/%li)\n",
				(long)(availsum / samples),
				(long)(delaysum / samples),
				(long)minavail, (long)mindelay,
				(long)maxavail, (long)maxdelay,
				(long)buffer_frames,
				counter, badavail, baddelay);
			tmr = now;
		}
	}
}

static ssize_t pcm_read(u_char *data, size_t rcount)
{
	ssize_t r;
	size_t result = 0;
	size_t count = rcount;

	if (count != chunk_size) {
		count = chunk_size;
	}

	while (count > 0 && !in_aborting) {
		if (test_position)
			do_test_position();
		check_stdin();
		r = readi_func(handle, data, count);
		if (test_position)
			do_test_position();
		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
			if (!test_nowait)
				snd_pcm_wait(handle, 100);
		} else if (r == -EPIPE) {
			xrun();
		} else if (r == -ESTRPIPE) {
			suspend();
		} else if (r < 0) {
			printf("read error: %s", snd_strerror(r));
			prg_exit(EXIT_FAILURE);
		}
		if (r > 0) {
			result += r;
			count -= r;
			data += r * bits_per_frame / 8;
		}
	}
	return rcount;
}


/* calculate the data count to read from/to dsp */
static off64_t calc_count(void)
{
	off64_t count;

	if (timelimit == 0)
		if (sampleslimit == 0)
			count = pbrec_count;
		else
			count = snd_pcm_format_size(hwparams.format, sampleslimit * hwparams.channels);
	else {
		count = snd_pcm_format_size(hwparams.format, hwparams.rate * hwparams.channels);
		count *= (off64_t)timelimit;
	}
	return count < pbrec_count ? count : pbrec_count;
}


static void capture()
{
	off64_t count, rest;		/* number of bytes to capture */
	int audio_pos=0;
	int flag_pos=0;

	/* get number of bytes to capture */
	count = calc_count();
	if (count == 0)
		count = LLONG_MAX;

	/* WAVE-file should be even (I'm not sure), but wasting one byte
	   isn't a problem (this can only be in 8 bit mono) */
	if (count < LLONG_MAX)
		count += count % 2;
	else
		count -= count % 2;

	/* display verbose output to console */
	//header(file_type, name);

	/* setup sound hardware */
	set_params();

	init_stdin();

	do {
		rest = count;

		/* capture */
		while (rest > 0 && recycle_capture_file == 0 && !in_aborting) {
			size_t c = (rest <= (off64_t)chunk_bytes) ?
				(size_t)rest : chunk_bytes;
			size_t f = c * 8 / bits_per_frame;
			if (pcm_read(audiobuf, f) != f) {
				in_aborting = 1;
				break;
			}
			
			memcpy((((char *)InputData)+audio_pos), audiobuf, c);
			
			Record_flag[flag_pos] = true;
			//cout << "flag[" << flag_pos << "]" << Record_flag[flag_pos] << ": C=" << c << endl;
			flag_pos++;			
			count -= c;
			rest -= c;
			audio_pos += c;			
							
			if(audio_pos>=(ddr_buffer*chunk_bytes*50)){
				audio_pos = 0;
				flag_pos = 0;
			}

		}

		/* re-enable SIGUSR1 signal */
		if (recycle_capture_file) {
			recycle_capture_file = 0;
			signal(SIGUSR1, signal_handler_recycle);
		}


		if (in_aborting)
			prg_exit(EXIT_FAILURE);

		/* repeat the loop when format is raw without timelimit or
		 * requested counts of data are recorded
		 */
	} while ((file_type == FORMAT_RAW && !timelimit && !sampleslimit) || count > 0);
}

void Capture_Audio()
{	char *pcm_name = "default";
	int err;
	snd_pcm_info_t *info;

	#ifdef ENABLE_NLS
		setlocale(LC_ALL, "");
		textdomain(PACKAGE);
	#endif

	snd_pcm_info_alloca(&info);

	file_type = FORMAT_RAW;
	stream = SND_PCM_STREAM_CAPTURE;
	command = "arecord";
	start_delay = 1;
	rhwparams.format = SND_PCM_FORMAT_S16_LE;
	rhwparams.rate = 16000;
	rhwparams.channels = 1;
	//pcm_name = "sysdefault:CARD=MS";
	timelimit = 0;

	err = snd_pcm_open(&handle, pcm_name, stream, open_mode);
	if (err < 0) {
		printf("audio open error: %s", snd_strerror(err));
		return 1;
	}

	if ((err = snd_pcm_info(handle, info)) < 0) {
		printf("info error: %s", snd_strerror(err));
		return 1;
	}

	chunk_size = 2*window_size;
	hwparams = rhwparams;

	audiobuf = (u_char *)malloc(chunk_size);
	if (audiobuf == NULL) {
		printf("not enough memory");
		return 1;
	}

	writei_func = snd_pcm_writei;
	readi_func = snd_pcm_readi;
	writen_func = snd_pcm_writen;
	readn_func = snd_pcm_readn;

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGABRT, signal_handler);
	signal(SIGUSR1, signal_handler_recycle);
	
	capture();
	
	snd_pcm_close(handle);
	handle = NULL;
	free(audiobuf);
	free(InputData);
	free(Record_flag);
	snd_config_update_free_global();	
	prg_exit(EXIT_SUCCESS);
	
}