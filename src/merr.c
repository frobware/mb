#include "merr.h"
#include <math.h>
#include <string.h>

pthread_mutex_t stderr_lock = PTHREAD_MUTEX_INITIALIZER;
static void (*die_cb)() = NULL;
static int suppress = s_none;

void merr_suppress(int s) {
  suppress = s;
}

void die_set_cb(void (*cb)(int)) {
  die_cb = cb;
}

void fprintfp(FILE *fd, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);

  pthread_mutex_lock(&stderr_lock);
  vfprintf(fd, fmt, ap);
  pthread_mutex_unlock(&stderr_lock);

  va_end(ap);
}

void die(int err, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);

  pthread_mutex_lock(&stderr_lock);
  fputs("die: ", stderr);
  vfprintf(stderr, fmt, ap);
  pthread_mutex_unlock(&stderr_lock);
  if (die_cb) die_cb();

  va_end(ap);
  exit(err);
}

static void current_time(char *s, size_t sz, const char *strftime_fmt,
                         int *milliseconds) {
  struct tm *tm_info;
  struct timeval tv;

  gettimeofday(&tv, NULL);
  *milliseconds = lrint(tv.tv_usec / 1000.0);

  if (*milliseconds >= 1000) {
    *milliseconds -= 1000;
    tv.tv_sec++;
  }

  tm_info = localtime(&tv.tv_sec);
  strftime(s, sz, strftime_fmt, tm_info);
}

void error(const char *fmt, ...) {
  char time_buffer[1024];
  int milliseconds;
  va_list ap;

  if (suppress >= s_error) return;

  va_start(ap, fmt);

  current_time(time_buffer, sizeof(time_buffer) - 1, "%d/%b/%Y:%H:%M:%S", &milliseconds);
  pthread_mutex_lock(&stderr_lock);
  fprintf(stderr, "[%s.%.03d] ", time_buffer, milliseconds);
  fputs("error: ", stderr);
  vfprintf(stderr, fmt, ap);
  pthread_mutex_unlock(&stderr_lock);

  va_end(ap);
}

void warning(const char *fmt, ...) {
  va_list ap;

  if (suppress >= s_warning) return;

  va_start(ap, fmt);

  pthread_mutex_lock(&stderr_lock);
  fputs("warning: ", stderr);
  vfprintf(stderr, fmt, ap);
  pthread_mutex_unlock(&stderr_lock);

  va_end(ap);
}

void info(const char *fmt, ...) {
  va_list ap;

  if (suppress >= s_info) return;

  va_start(ap, fmt);

  pthread_mutex_lock(&stderr_lock);
  fputs("info: ", stderr);
  vfprintf(stderr, fmt, ap);
  pthread_mutex_unlock(&stderr_lock);

  va_end(ap);
}

void dbg(const char *fmt, ...) {
  va_list ap;
  struct timeval tv;
  gettimeofday(&tv,NULL);

  va_start(ap, fmt);

  pthread_mutex_lock(&stderr_lock);
  fprintf(stderr, "%ld.%ld: ", tv.tv_sec, tv.tv_usec);
  fputs("dbg: ", stderr);
  vfprintf(stderr, fmt, ap);
  pthread_mutex_unlock(&stderr_lock);

  va_end(ap);
}
