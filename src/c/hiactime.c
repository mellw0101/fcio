/** @file hiactime.c

  @author  Melwin Svensson.
  @date    16-7-2025.

  High accuracy time.

 */
#include "../include/proto.h"


/* ---------------------------------------------------------- Define's ---------------------------------------------------------- */


/* Interval time. */
#define INTERVAL_0  (1000000LL)
#define INTERVAL_1  (300000LL)
#define INTERVAL_2  (100000LL)

/* Jitter forgiveness time. */
#define JITTER_0  (300000LL)
#define JITTER_1  (150000LL)
#define JITTER_2  (60000LL)

/* Segment sleep. */
#define DO_SLEEP_SEGMENT(elapsed, total, start, now, stage)            \
  while (((elapsed) + INTERVAL_##stage + JITTER_##stage) < (total)) {  \
    nanosleep(&timespec_##stage, NULL);                                \
    clock_gettime(CLOCK_MONOTONIC, (now));                             \
    (elapsed) = TIMESPEC_ELAPSED_NS((start), (now));                   \
  }


/* ---------------------------------------------------------- Variable's ---------------------------------------------------------- */


/* Timespec segments. */
static const struct timespec timespec_0 = {0, INTERVAL_0};
static const struct timespec timespec_1 = {0, INTERVAL_1};
static const struct timespec timespec_2 = {0, INTERVAL_2};


/* ---------------------------------------------------------- Global function's ---------------------------------------------------------- */


/* ----------------------------- Hiactime sleep total duration ----------------------------- */

/* Based on how mush time has elapsed from `s`, sleep until we achive the exact time in nanoseconds.
 * `e` does not need to gathered by the caller, rather can be used to measure
 * the full sleep time after the call.  Note that for milli-second accuracy this is basicly perfect. */
void hiactime_sleep_total_duration(const struct timespec *const s, struct timespec *const e, Llong nanoseconds) {
  long elapsed;
  /* Get the current time. */
  clock_gettime(CLOCK_MONOTONIC, e);
  elapsed = TIMESPEC_ELAPSED_NS(s, e);
  /* Perform all sleep stages. */
  DO_SLEEP_SEGMENT(elapsed, nanoseconds, s, e, 0);
  DO_SLEEP_SEGMENT(elapsed, nanoseconds, s, e, 1);
  DO_SLEEP_SEGMENT(elapsed, nanoseconds, s, e, 2);
  /* For the remaining time, spin the time away. */
  while (TIMESPEC_ELAPSED_NS(s, e) < nanoseconds) {
    clock_gettime(CLOCK_MONOTONIC, e);
  }
}

/* ----------------------------- Hiactime nsleep ----------------------------- */

/* High-accuracy `nano-second` sleep. */
void hiactime_nsleep(Llong nanoseconds) {
  struct timespec s;
  struct timespec e;
  clock_gettime(CLOCK_MONOTONIC, &s);
  hiactime_sleep_total_duration(&s, &e, nanoseconds);
}

/* ----------------------------- Hiactime nsleep ----------------------------- */

/* High-accuracy `milli-second` sleep. */
void hiactime_msleep(double milliseconds) {
  hiactime_nsleep(MILLI_TO_NANO(milliseconds));
}
