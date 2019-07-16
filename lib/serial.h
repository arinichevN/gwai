
#ifndef LIBPAS_SERIAL_H
#define LIBPAS_SERIAL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>

#include "timef.h"

extern int serial_open(const char *device, const int baud, const char *config);

extern int serial_init(int *fd, const char *device, const int baud, const char *config);

extern void serial_printOptions (const int fd);

extern int serial_puts(const int fd, char *str);

extern int serial_canRead(int fd, int timeout_ms);

extern int serial_canWrite(int fd, int timeout_ms);

extern size_t serial_read(int fd, void *buf, size_t buf_size);

extern size_t serial_readUntil(int fd, char *buf, size_t buf_size, char end);

extern void serial_readAll(int fd);
#endif

