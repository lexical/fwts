/*
 * Copyright (C) 2006, Intel Corporation
 * Copyright (C) 2010-2014 Canonical
 *
 * This file was originally part of the Linux-ready Firmware Developer Kit
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#define _GNU_SOURCE

#include "fwts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <dirent.h>
#include <stdint.h>
#include <stdbool.h>
#include <sched.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <inttypes.h>

#define FWTS_CPU_PATH	"/sys/devices/system/cpu"

#define MAX_FREQS	256

typedef struct {
	uint64_t	Hz;
	uint64_t	speed;
} fwts_cpu_freq;

static int number_of_speeds = -1;
static int total_tests = 1;
static int performed_tests = 0;
static bool no_cpufreq = false;
static uint64_t top_speed = 0;
static int num_cpus;

#define GET_PERFORMANCE_MAX (0)
#define GET_PERFORMANCE_MIN (1)
#define GET_PERFORMANCE_AVG (2)

#define MAX_ABSOLUTE_ERROR	20.0		/* In Hz */
#define MAX_RELATIVE_ERROR	0.0025		/* as fraction */

/*
 *  hz_almost_equal()
 *	used to compare CPU _PSS levels, are they almost
 *	equal?  E.g. within MAX_ABSOLUTE_ERROR Hz difference
 *	between each other, or a relative difference of
 *	MAX_RELATIVE_ERROR.  If they are, then they are deemed
 *	almost equal.
 */
static int hz_almost_equal(const uint64_t a, const uint64_t b)
{
	double da = (double)a, db = (double)b;
	double relative_error, abs_diff = fabs(da - db);

	if (a == b)
		return true;
	if (abs_diff < MAX_ABSOLUTE_ERROR)
		return true;
	if (db > da)
		relative_error = abs_diff / db;
	else
		relative_error = abs_diff / da;

	return relative_error <= MAX_RELATIVE_ERROR;
}

static inline void cpu_mkpath(
	char *const path,
	const int len,
	const int cpu,
	const char *const name)
{
	snprintf(path, len, "%s/cpu%i/cpufreq/%s", FWTS_CPU_PATH, cpu, name);
}

static void set_governor(fwts_framework *fw, const int cpu)
{
	char path[PATH_MAX];

	cpu_mkpath(path, sizeof(path), cpu, "scaling_governor");

	if (fwts_set("userspace", path) != FWTS_OK) {
		if (!no_cpufreq) {
			fwts_warning(fw,
				"Cannot set CPU scaling governor to userspace scaling.");
			no_cpufreq = true;
		}
	}
}

#ifdef FWTS_ARCH_INTEL
static int cpu_exists(const int cpu)
{
	char path[PATH_MAX];

	cpu_mkpath(path, sizeof(path), cpu, "scaling_governor");
	return !access(path, R_OK);
}
#endif

static void set_HZ(fwts_framework *fw, const int cpu, const uint64_t Hz)
{
	cpu_set_t mask, oldset;
	char path[PATH_MAX];
	char buffer[64];

	/* First, go to the right cpu */

	sched_getaffinity(0, sizeof(oldset), &oldset);

	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	sched_setaffinity(0, sizeof(mask), &mask);

	set_governor(fw, cpu);

	/* then set the speed */
	cpu_mkpath(path, sizeof(path), cpu, "scaling_setspeed");
	snprintf(buffer, sizeof(buffer), "%" PRIu64 , Hz);
	fwts_set(buffer, path);

	sched_setaffinity(0, sizeof(oldset), &oldset);
}

#ifdef FWTS_ARCH_INTEL
static int get_performance_repeat(
	fwts_framework *fw,
	const int cpu,
	const int count,
	const int type,
	uint64_t *retval)
{
	int i;

	uint64_t max = 0;
	uint64_t min = ~0;
	uint64_t real_count = 0;
	uint64_t cumulative = 0;

	for (i = 0; i < count; i++) {
		uint64_t temp;

		if (fwts_cpu_performance(fw, cpu, &temp) != FWTS_OK)
			return FWTS_ERROR;

		if (temp) {
			if (temp < min)
				min = temp;

			if (temp > max)
				max = temp;

			cumulative += temp;
			real_count++;
		}
	}

	switch (type) {
	case GET_PERFORMANCE_MAX:
		*retval = max;
		break;
	case GET_PERFORMANCE_MIN:
		*retval = min;
		break;
	case GET_PERFORMANCE_AVG:
		if (real_count)
			*retval = cumulative / real_count;
		else
			*retval = 0;
		break;
	default:
		*retval = 0;
		break;
	}
	return FWTS_OK;
}
#endif

static char *hz_to_human(const uint64_t hz)
{
	static char buffer[32];

	if (hz > 1000000) {
		snprintf(buffer, sizeof(buffer), "%6.3f GHz",
			(double)hz / 1000000.0);
		return buffer;
	} else if (hz > 1000) {
		snprintf(buffer, sizeof(buffer), "%6.3f MHz",
			(double)hz / 1000.0);
		return buffer;
	} else {
		snprintf(buffer, sizeof(buffer), "%" PRIu64 " Hz", hz);
		return buffer;
	}
}

static uint64_t get_claimed_hz(const int cpu)
{
	char path[PATH_MAX];
	char *buffer;
	uint64_t value = 0;

	cpu_mkpath(path, sizeof(path), cpu, "scaling_max_freq");

	if ((buffer = fwts_get(path)) != NULL) {
		value = strtoul(buffer, NULL, 10);
		free(buffer);
	}
	return value;
}

static uint64_t get_bios_limit(const int cpu)
{
	char path[PATH_MAX];
	char *buffer;
	uint64_t value = 0;

	cpu_mkpath(path, sizeof(path), cpu, "bios_limit");

	if ((buffer = fwts_get(path)) != NULL) {
		value = strtoul(buffer, NULL, 10);
		free(buffer);
	}
	return value;
}

static int cpu_freq_compare(const void *v1, const void *v2)
{
	const fwts_cpu_freq *cpu_freq1 = (fwts_cpu_freq *)v1;
	const fwts_cpu_freq *cpu_freq2 = (fwts_cpu_freq *)v2;

	/*
	 * Some _PSS states can be the same or very nearly
	 * the same when Turbo mode is available,
	 * so if they are we also differentiate the two by
	 * the speed to get a fully sorted ordering
	 */
	if (hz_almost_equal(cpu_freq1->Hz, cpu_freq2->Hz))
		return cpu_freq1->speed - cpu_freq2->speed;
	else
		return cpu_freq1->Hz - cpu_freq2->Hz;
}

static int read_freqs_available(const int cpu, fwts_cpu_freq *freqs)
{
	char path[PATH_MAX];
	char line[4096];
	FILE *file;
	char *c, *c2;
	int i = 0;

	memset(line, 0, sizeof(line));
	cpu_mkpath(path, sizeof(path), cpu, "scaling_available_frequencies");
	if ((file = fopen(path, "r")) == NULL)
		return 0;
	c = fgets(line, 4095, file);
	fclose(file);
	if (!c)
		return 0;

	while ((i < MAX_FREQS) && c && strlen(c) > 1) {
		c2 = strchr(c, ' ');
		if (c2) {
			*c2 = 0;
			c2++;
		} else
			c2 = NULL;

		freqs[i].Hz = strtoull(c, NULL, 10);
		c = c2;
		i++;
	}
	return i;
}

static void do_cpu(fwts_framework *fw, const int cpu)
{
	fwts_cpu_freq freqs[MAX_FREQS];
	int i, speedcount;
	static int warned = 0;
	bool warned_PSS = false;
	uint64_t cpu_top_speed = 1;
	int claimed_hz_too_low = 0;
	int bios_limit_too_low = 0;
	const uint64_t claimed_hz = get_claimed_hz(cpu);
	const uint64_t bios_limit = get_bios_limit(cpu);

	memset(freqs, 0, sizeof(freqs));
	set_governor(fw, cpu);

	if ((speedcount = read_freqs_available(cpu, freqs)) == 0) {
		if (!no_cpufreq) {
			char path[PATH_MAX];
			char *driver;

			no_cpufreq = true;
			fwts_warning(fw, "CPU frequency scaling not supported.");

			cpu_mkpath(path, sizeof(path), cpu, "scaling_driver");
			driver = fwts_get(path);
			if (driver) {
				fwts_advice(fw, "Scaling driver '%s' is enabled and this "
					"does not seem to allow CPU frequency scaling.", driver);
				free(driver);
			}
		}
		return;
	}
	if (total_tests == 1)
		total_tests = ((2 + speedcount) * num_cpus) + 4;

	for (i = 0; i < speedcount; i++) {
		set_HZ(fw, cpu, freqs[i].Hz);

		if ((claimed_hz != 0) && (claimed_hz < freqs[i].Hz))
			claimed_hz_too_low++;
		if ((bios_limit != 0) && (bios_limit < freqs[i].Hz))
			bios_limit_too_low++;

		if (fwts_cpu_performance(fw, cpu, &freqs[i].speed) != FWTS_OK) {
			fwts_log_error(fw, "Failed to get CPU performance for "
				"CPU frequency %" PRIu64 " Hz.", freqs[i].Hz);
			freqs[i].speed = 0;
		}
		if (freqs[i].speed > cpu_top_speed)
			cpu_top_speed = freqs[i].speed;

		performed_tests++;
		fwts_progress(fw, 100 * performed_tests/total_tests);
	}

	if (cpu_top_speed > top_speed)
		top_speed = cpu_top_speed;

	if (claimed_hz_too_low) {
		char path[PATH_MAX];

		cpu_mkpath(path, sizeof(path), cpu, "scaling_max_freq");
		fwts_warning(fw,
			"There were %d CPU frequencies larger than the _PSS "
			"maximum CPU frequency of %s for CPU %d. Has %s "
			"been set too low?",
			claimed_hz_too_low, hz_to_human(claimed_hz), cpu, path);
	}

	if (bios_limit_too_low) {
		char path[PATH_MAX];

		cpu_mkpath(path, sizeof(path), cpu, "bios_limit");
		fwts_warning(fw,
			"The CPU frequency BIOS limit %s for CPU %d was set to %s "
			"which is lower than some of the ACPI scaling frequencies.",
			path, cpu, hz_to_human(bios_limit));
	}

	if (claimed_hz_too_low || bios_limit_too_low)
		fwts_log_nl(fw);

	fwts_log_info(fw, "CPU %d: %i CPU frequency steps supported.", cpu, speedcount);
	fwts_log_info_verbatum(fw, " Frequency | Relative Speed | Bogo loops");
	fwts_log_info_verbatum(fw, "-----------+----------------+-----------");
	for (i = 0; i < speedcount; i++) {
		char *turbo = "";
#ifdef FWTS_ARCH_INTEL
		if ((i == 0) && (speedcount > 1) &&
		    (hz_almost_equal(freqs[i].Hz, freqs[i + 1].Hz)))
			turbo = " (Turbo Boost)";
#endif

		fwts_log_info_verbatum(fw, "%10s |     %5.1f %%    | %9" PRIu64 "%s",
			hz_to_human(freqs[i].Hz),
			100.0 * freqs[i].speed/cpu_top_speed,
			freqs[i].speed,
			turbo);
	}

	if (number_of_speeds == -1)
		number_of_speeds = speedcount;

	fwts_log_nl(fw);

	if (number_of_speeds != speedcount)
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"CPUFreqPStates",
			"Not all processors support the same number of P states.");

	if (speedcount < 2)
		return;

	/* Sort the frequencies */
	qsort(freqs, speedcount, sizeof(fwts_cpu_freq), cpu_freq_compare);

	/* now check for 1) increasing HZ and 2) increasing speed */
	for (i = 0; i < speedcount-1; i++) {
		if (freqs[i].Hz == freqs[i+1].Hz && !warned++)
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"CPUFreqDupFreq",
				"Duplicate frequency reported.");
		if (freqs[i].speed > freqs[i+1].speed)
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"CPUFreqSlowerOnCPU",
				"Supposedly higher frequency %s is slower (%" PRIu64
				" bogo loops) than frequency %s (%" PRIu64
				" bogo loops) on CPU %i.",
				hz_to_human(freqs[i+1].Hz), freqs[i+1].speed,
				hz_to_human(freqs[i].Hz), freqs[i].speed,
				cpu);

		if ((freqs[i].Hz > claimed_hz) && !warned_PSS) {
			warned_PSS = true;
			fwts_warning(fw, "Frequency %" PRIu64
				" not achievable; _PSS limit of %" PRIu64 " in effect?",
				freqs[i].Hz, claimed_hz);
		}
	}
}


static void lowest_speed(fwts_framework *fw, const int cpu)
{
	char path[PATH_MAX];
	char *line;
	char *c, *c2;
	uint64_t Hz, lowspeed = 0;

	cpu_mkpath(path, sizeof(path), cpu, "scaling_available_frequencies");
	if ((line = fwts_get(path)) == NULL)
		return;

	c = line;
	while (c && strlen(c) > 1) {
		c2 = strchr(c, ' ');
		if (c2) {
			*c2 = 0;
			c2++;
		} else
			c2 = NULL;

		Hz = strtoull(c, NULL, 10);
		if (Hz < lowspeed || lowspeed == 0)
			lowspeed = Hz;
		c = c2;
	}
	free(line);

	set_HZ(fw, cpu, lowspeed);
}

static void highest_speed(fwts_framework *fw, const int cpu)
{
	char path[PATH_MAX];
	char *line;
	uint64_t Hz;
	char *c, *c2;
	unsigned long highspeed=0;

	cpu_mkpath(path, sizeof(path), cpu, "scaling_available_frequencies");
	if ((line = fwts_get(path)) == NULL)
		return;

	c = line;
	while (c && strlen(c) > 1) {
		c2 = strchr(c, ' ');
		if (c2) {
			*c2=0;
			c2++;
		} else
			c2 = NULL;

		Hz = strtoull(c, NULL, 10);
		if (Hz > highspeed || highspeed == 0)
			highspeed = Hz;
		c = c2;
	}
	free(line);

	set_HZ(fw, cpu, highspeed);
}


#ifdef FWTS_ARCH_INTEL
/*
 * 4) Is BIOS wrongly doing Sw_All P-state coordination across cpus
 * - Change frequency on all CPU to the lowest value
 * - Change frequency on one particular CPU to the highest
 * - If BIOS is doing Sw_All, the last high freq request will not work
 */
static void do_sw_all_test(fwts_framework *fw)
{
	DIR *dir;
	struct dirent *entry;
	uint64_t highperf, lowperf;
	int first_cpu_index = -1;
	int cpu;
	int ret;

	if ((dir = opendir(FWTS_CPU_PATH)) == NULL) {
		fwts_log_error(fw, "FATAL: cpufreq: sysfs not mounted.");
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (entry && strlen(entry->d_name) > 3) {
			cpu = strtoul(entry->d_name + 3, NULL, 10);
			if (first_cpu_index == -1)
				first_cpu_index = cpu;

			lowest_speed(fw, cpu);
		}
	}
	closedir(dir);

	/* All CPUs at the lowest frequency */
	ret = get_performance_repeat(fw, first_cpu_index, 5, GET_PERFORMANCE_MIN, &lowperf);
	performed_tests++;
	fwts_progress(fw, 100 * performed_tests/total_tests);
	if (ret != FWTS_OK) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "CPUFreqSW_ALLGetPerf",
			"Failed to get CPU performance.");
		return;
	}
	lowperf = (lowperf * 100) / top_speed;

	highest_speed(fw, first_cpu_index);
	ret = get_performance_repeat(fw, first_cpu_index, 5, GET_PERFORMANCE_MAX, &highperf);
	performed_tests++;
	fwts_progress(fw, 100 * performed_tests/total_tests);
	if (ret != FWTS_OK) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "CPUFreqSW_ALLGetPerf",
			"Failed to get CPU performance.");
		return;
	}
	highperf = (highperf * 100) / top_speed;

	if (lowperf >= highperf)
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"CPUFreqSW_ALL",
			"Firmware not implementing hardware "
			"coordination cleanly. Firmware using SW_ALL "
			"instead?");
}


/*
 * 5) Is BIOS wrongly doing Sw_Any P-state coordination across cpus
 * - Change frequency on all CPU to the lowest value
 * - Change frequency on one particular CPU to the highest
 * - Change frequency on all CPU to the lowest value
 * - If BIOS is doing Sw_Any, the high freq request will not work
 */
static void do_sw_any_test(fwts_framework *fw)
{
	DIR *dir;
	struct dirent *entry;
	uint64_t highperf, lowperf;
	int first_cpu_index = -1;
	int cpu;
	int ret;

	if ((dir = opendir(FWTS_CPU_PATH)) == NULL) {
		fwts_log_error(fw, "FATAL: cpufreq: sysfs not mounted.");
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (entry && strlen(entry->d_name) > 3) {
			cpu = strtoul(entry->d_name + 3, NULL, 10);
			if (first_cpu_index == -1)
				first_cpu_index = cpu;

			lowest_speed(fw, cpu);
		}
	}
	rewinddir(dir);

	/* All CPUs at the lowest frequency */
	ret = get_performance_repeat(fw, first_cpu_index, 5, GET_PERFORMANCE_MIN, &lowperf);
	performed_tests++;
	fwts_progress(fw, 100 * performed_tests/total_tests);
	if (ret != FWTS_OK) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "CPUFreqSW_ANYGetPerf",
			"Failed to get CPU performance.");
		closedir(dir);
		return;
	}
	lowperf = (100 * lowperf) / top_speed;

	highest_speed(fw, first_cpu_index);

	while ((entry = readdir(dir)) != NULL) {
		if (entry && strlen(entry->d_name) > 3) {
			cpu = strtoul(entry->d_name + 3, NULL, 10);
			if (cpu == first_cpu_index)
				continue;
			lowest_speed(fw, cpu);
		}
	}
	closedir(dir);

	ret = get_performance_repeat(fw, first_cpu_index, 5, GET_PERFORMANCE_MAX, &highperf);
	performed_tests++;
	fwts_progress(fw, 100 * performed_tests/total_tests);
	if (ret != FWTS_OK) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "CPUFreqSW_ANYGetPerf",
			"Failed to get CPU performance.");
		return;
	}
	highperf = (100 * highperf) / top_speed;

	if (lowperf >= highperf)
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"CPUFreqSW_ANY",
			"Firmware not implementing hardware "
			"coordination cleanly. Firmware using SW_ANY "
			"instead?.");
}

static void check_sw_any(fwts_framework *fw)
{
	DIR *dir;
	struct dirent *entry;
	uint64_t low_perf, high_perf, newhigh_perf;
	static int once = 0;
	int max_cpu = 0, i, j;
	int cpu;

	/* First set all processors to their lowest speed */
	if ((dir = opendir(FWTS_CPU_PATH)) == NULL) {
		fwts_log_error(fw, "FATAL: cpufreq: sysfs not mounted.");
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (entry && strlen(entry->d_name) > 3) {
			cpu = strtoul(entry->d_name + 3, NULL, 10);
			lowest_speed(fw, cpu);
			if (cpu > max_cpu)
				max_cpu = cpu;
		}
	}
	closedir(dir);

	if (max_cpu == 0)
		return; /* Single processor machine, no point in checking anything */

	/* assume that all processors have the same low performance */
	if (fwts_cpu_performance(fw, max_cpu, &low_perf) != FWTS_OK) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"CPUFreqCPsSetToSW_ANYGetPerf",
			"Cannot get CPU performance.");
		return;
	}

	for (i = 0; i <= max_cpu; i++) {
		highest_speed(fw, i);
		if (!cpu_exists(i))
			continue;

		if (fwts_cpu_performance(fw, i, &high_perf) != FWTS_OK) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"CPUFreqCPsSetToSW_ANYGetPerf",
				"Cannot get CPU performance.");
			return;
		}
		performed_tests++;
		fwts_progress(fw, 100 * performed_tests/total_tests);
		/*
		 * now set all the others to low again; sw_any will cause
		 * the core in question to now also get the low speed, while
		 * hardware max will keep the performance
		 */
		for (j = 0; j <= max_cpu; j++)
			if (i != j)
				lowest_speed(fw, j);
		if (fwts_cpu_performance(fw, i, &newhigh_perf) != FWTS_OK) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"CPUFreqCPsSetToSW_ANYGetPerf",
				"Cannot get CPU performance.");
			return;
		}
		if ((high_perf > newhigh_perf) &&
		    (high_perf - newhigh_perf > (high_perf - low_perf)/4) &&
		    (once == 0) && (high_perf - low_perf > 20)) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"CPUFreqCPUsSetToSW_ANY",
				"Processors are set to SW_ANY.");
			once++;
			lowest_speed(fw, i);
		}
		performed_tests++;
		fwts_progress(fw, 100 * performed_tests/total_tests);
	}
	if (!once)
		fwts_passed(fw, "P-state coordination under hardware control.");
}
#endif

static int cpufreq_test1(fwts_framework *fw)
{
	DIR *dir;
	struct dirent *entry;
	int cpu;

#ifdef FWTS_ARCH_INTEL
	fwts_log_info(fw,
		"For each processor in the system, this test steps through the "
		"various frequency states (P-states) that the BIOS advertises "
		"for the processor. For each processor/frequency combination, "
		"a quick performance value is measured. The test then validates that:");
	fwts_log_info_verbatum(fw, "  1. Each processor has the same number of frequency states.");
	fwts_log_info_verbatum(fw, "  2. Higher advertised frequencies have a higher performance.");
	fwts_log_info_verbatum(fw, "  3. No duplicate frequency values are reported by the BIOS.");
	fwts_log_info_verbatum(fw, "  4. BIOS doing Sw_All P-state coordination across cores.");
	fwts_log_info_verbatum(fw, "  5. BIOS doing Sw_Any P-state coordination across cores.");
#else
	fwts_log_info(fw,
		"For each processor in the system, this test steps through the "
		"various frequency states that the CPU supports. "
		"For each processor/frequency combination, "
		"a quick performance value is measured. The test then validates that:");
	fwts_log_info_verbatum(fw, "  1. Each processor has the same number of frequency states.");
	fwts_log_info_verbatum(fw, "  2. Higher advertised frequencies have a higher performance.");
	fwts_log_info_verbatum(fw, "  3. No duplicate frequency values exist.");
#endif
	fwts_log_nl(fw);

	/* First set all processors to their lowest speed */
	if ((dir = opendir(FWTS_CPU_PATH)) == NULL) {
		fwts_log_error(fw, "FATAL: cpufreq: sysfs not mounted\n");
		return FWTS_ERROR;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (entry && strlen(entry->d_name) > 3 && isdigit(entry->d_name[3])) {
			cpu = strtoul(entry->d_name + 3, NULL, 10);
			lowest_speed(fw, cpu);
		}
	}
	rewinddir(dir);

	/* then do the benchmark */

	while ((entry = readdir(dir)) != NULL) {
		if (entry && strlen(entry->d_name) > 3 && isdigit(entry->d_name[3])) {
			cpu = strtoul(entry->d_name + 3, NULL, 10);
			do_cpu(fw, cpu);
			lowest_speed(fw, cpu);
			if (no_cpufreq)
				break;
		}
	}
	rewinddir(dir);

	/* set everything back to the highest speed again */

	while ((entry = readdir(dir)) != NULL) {
		if (entry && strlen(entry->d_name) > 3 && isdigit(entry->d_name[3])) {
			cpu = strtoul(entry->d_name + 3, NULL, 10);
			highest_speed(fw, cpu);
		}
	}
	closedir(dir);

#ifdef FWTS_ARCH_INTEL
	if (!no_cpufreq)
		check_sw_any(fw);

	/*
	 * Check for more than one CPU and more than one frequency and
	 * then do the benchmark set 2
	 */
	if (num_cpus > 1 && number_of_speeds > 1) {
		do_sw_all_test(fw);
		do_sw_any_test(fw);
	} else if (number_of_speeds > 1) {
		performed_tests += 4;
		fwts_progress(fw, 100 * performed_tests/total_tests);
	}
#endif
	fwts_progress(fw, 100);

	return FWTS_OK;
}

static int cpufreq_init(fwts_framework *fw)
{
	if ((num_cpus = fwts_cpu_enumerate()) == FWTS_ERROR) {
		fwts_warning(fw, "Cannot determine number of CPUS, defaulting to 1.");
		num_cpus = 1;
	}
	return FWTS_OK;
}

static fwts_framework_minor_test cpufreq_tests[] = {
#ifdef FWTS_ARCH_INTEL
	{ cpufreq_test1, "CPU P-State tests." },
#else
	{ cpufreq_test1, "CPU Frequency tests." },
#endif
	{ NULL, NULL }
};

static fwts_framework_ops cpufreq_ops = {
	.init        = cpufreq_init,
	.description = "CPU frequency scaling tests.",
	.minor_tests = cpufreq_tests
};

FWTS_REGISTER("cpufreq", &cpufreq_ops, FWTS_TEST_ANYTIME, FWTS_FLAG_BATCH | FWTS_FLAG_ROOT_PRIV)
