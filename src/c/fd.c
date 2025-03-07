/** @file fd.c

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#include "../include/proto.h"


/* Lock a file descriptor. */
void fdlock(int fd, short type) {
  struct flock lock = {0};
  /* Set flock field's. */
  lock.l_type   = type;
  lock.l_whence = SEEK_SET;
  lock.l_pid    = getpid();
  /* Perform the locking. */
  ALWAYS_ASSERT(fcntl(fd, F_SETLKW, &lock) != -1);
}

/* Unlock a file desctiptor. */
void fdunlock(int fd) {
  struct flock lock = {0};
  /* Set flock fields. */
  lock.l_type   = F_UNLCK;
  lock.l_whence = SEEK_SET;
  lock.l_pid    = getpid();
  /* Perform the unlocking. */
  ALWAYS_ASSERT(fcntl(fd, F_SETLK, &lock) != -1);
}

/* Disable canonical mode and echo for `fd`. */
void disable_canonecho(int fd, struct termios *const oldt) {
  ALWAYS_ASSERT(fd >= 0);
  ASSERT(oldt);
  struct termios newt;
  /* Save the current state of the terminal. */
  ALWAYS_ASSERT(tcgetattr(fd, oldt) != -1);
  newt = *oldt;
  /* Disable canonical mode and echo. */
  newt.c_lflag &= ~(ICANON | ECHO);
  ALWAYS_ASSERT(tcsetattr(fd, TCSANOW, &newt) != -1);
}

/* Restore `fd` to `t`. */
void restore_termios(int fd, struct termios *const t) {
  ALWAYS_ASSERT(fd >= 0);
  ASSERT(t);
  /* Restore termios settings. */
  ALWAYS_ASSERT(tcsetattr(fd, TCSANOW, t) != -1);
}

/* Set `flags` of `fd` and assign the original state to `*oldf`. */
void setfdflags(int fd, int *oldf, int flags) {
  ALWAYS_ASSERT(fd >= 0);
  ASSERT(oldf);
  /* Set fd to nonblocking mode. */
  ALWAYS_ASSERT((*oldf = fcntl(fd, F_GETFL, 0)) != -1);
  ALWAYS_ASSERT(fcntl(fd, F_SETFL, (*oldf | flags)) != -1);
}

/* Restore `fd` to state `*f`. */
void restfdflags(int fd, int *f) {
  ALWAYS_ASSERT(fd >= 0);
  ASSERT(f);
  ALWAYS_ASSERT(fcntl(fd, F_SETFL, *f) != -1);
}

