#ifndef	_TIME_H
#define _TIME_H

#if !defined(_DLIB_TIME_USES_64) || !defined(_DLIB_TIME_USES_LONG) || (_DLIB_TIME_USES_64 == 0) || (_DLIB_TIME_USES_LONG == 0)
#error "_DLIB_TIME_USES_64=1 and _DLIB_TIME_USES_LONG=1 should be defined in IAR project(C/C++ Compiler->Preprocessor->Defined symbols)."
#endif

#ifndef tm_isdst
#define tm_isdst    tm_isdst;	\
	long __tm_gmtoff;	\
	const char *__tm_zone

#if defined(_BSD_SOURCE) || defined(_GNU_SOURCE)
#define __tm_gmtoff tm_gmtoff
#define __tm_zone tm_zone
#endif

#include_next <time.h>
#undef tm_isdst

#else /* tm_isdst */

#include_next <time.h>

#endif /* tm_isdst */

#ifdef __cplusplus
extern "C" {
#endif

#define __NEED_timer_t
#define __NEED_clockid_t

#include <bits/alltypes.h>

struct itimerspec {
	struct timespec it_interval;
	struct timespec it_value;
};

#define CLOCK_REALTIME           0
#define CLOCK_MONOTONIC          1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID  3
#define CLOCK_MONOTONIC_RAW      4
#define CLOCK_REALTIME_COARSE    5
#define CLOCK_MONOTONIC_COARSE   6
#define CLOCK_BOOTTIME           7
#define CLOCK_REALTIME_ALARM     8
#define CLOCK_BOOTTIME_ALARM     9
#define CLOCK_SGI_CYCLE         10
#define CLOCK_TAI               11

#define TIMER_ABSTIME 1

struct tm *gmtime_r (const time_t *__restrict, struct tm *__restrict);
struct tm *localtime_r (const time_t *__restrict, struct tm *__restrict);

int nanosleep (const struct timespec *, struct timespec *);
int clock_getres (clockid_t, struct timespec *);
int clock_gettime (clockid_t, struct timespec *);
int clock_settime (clockid_t, const struct timespec *);
int clock_nanosleep (clockid_t, int, const struct timespec *, struct timespec *);

struct sigevent;
int timer_create (clockid_t, struct sigevent *__restrict, timer_t *__restrict);
int timer_delete (timer_t);
int timer_settime (timer_t, int, const struct itimerspec *__restrict, struct itimerspec *__restrict);
int timer_gettime (timer_t, struct itimerspec *);
int timer_getoverrun (timer_t);

#ifdef __cplusplus
}
#endif

#endif
