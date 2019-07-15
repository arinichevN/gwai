
#ifndef LIBPAS_PUART_H
#define LIBPAS_PUART_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

#include "serial.h"
#include "app.h"

#define PUART_DELIMITER_END ';'
#define PUART_DELIMITER_END_STR ";"
#define PUART_DELIMITER_BLOCK '_'
#define PUART_DELIMITER_BLOCK_STR "_"

#define PUART_STR_MAX_LENGTH 16

#define PUART_CONNECTION_FAILED -1
#define PUART_NO_RESPONSE -2
#define PUART_SUCCESS 1

#define puart_sendEnd(fd) serial_puts(fd, "end;")

extern int puart_readResponse(int fd, char *buf, int len);

extern int puart_sendDouble(int fd, int channel_id, char *cmd, double v);

extern int puart_sendInt(int fd, int channel_id, char *cmd, int v);

extern int puart_sendStr(int fd, int channel_id, char *cmd, char *v);

extern int puart_sendCmd (int fd, int channel_id, char *cmd);

extern int puart_channelExist (int fd, int channel_id);


#endif

