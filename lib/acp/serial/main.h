#ifndef LIBPAS_ACP_SERIAL_MAIN_H
#define LIBPAS_ACP_SERIAL_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

#include "../../debug.h"
#include "../main.h"
#include "../data.h"
#include "../../serial.h"

#define ACPSERIAL_CHECK_CRC 1
#define ACPSERIAL_TIMEOUT_MS 500

typedef struct {
    int id;
    double value;
    unsigned long tm;
    int state;
} SFTS;

extern int acpserial_checkCRC(const char *str);
extern int acpserial_checkChannelIdCRC(const char *str, int channel_id);
extern int acpserial_send ( const char *buf, int fd );
extern int acpserial_readResponse(int fd, char *buf, size_t len);
extern int acpserial_sendTcpPack(int fd, char *pack_str);

extern int acpserial_sendChCmd(int fd, char sign, int cmd, int channel_id);
extern int acpserial_sendChCmdF1(int fd, char sign, int cmd, int channel_id, double v);
extern int acpserial_sendChCmdI1(int fd, char sign, int cmd, int channel_id, int v);
extern int acpserial_sendChCmdStr(int fd, char sign, int cmd, int channel_id, const char *v);

extern int acpserial_extractFTS(const char *buf, size_t len, FTS *v);
extern int acpserial_extractI2(const char *buf, size_t len, int *v1, int *v2);
extern int acpserial_extractSI(const char *buf, size_t len, char *v1, size_t v1_len, int *v2);

#endif 

