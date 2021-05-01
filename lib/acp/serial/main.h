#ifndef LIBPAS_ACP_SERIAL_H
#define LIBPAS_ACP_SERIAL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

#include "../../serial.h"
#include "../../debug.h"
#include "../../app.h"
#include "../main.h"

#define ACPSERIAL_CHECK_CRC 1
#define ACPSERIAL_TIMEOUT_MS 500

extern int acpserial_packCheckCRC(const char *str);
extern int acpserial_checkChannelIdCRC(const char *str, int channel_id);
extern int acpserial_send(int fd, const char *buf);
extern int acpserial_readResponse(int fd, char *buf, size_t buf_len);
extern int acpserial_readRequest(int fd, char *buf, size_t buf_len);
extern int acpserial_tcpToSerial(char *buf, size_t buf_len);
extern int acpserial_sendTcpPack(int fd, char *pack_str);

extern int acpserial_buildPackSI(char *buf, size_t buf_max_len, char sign, const char *v1, int v2);
extern int acpserial_buildPackSII(char *buf, size_t buf_max_len, char sign, const char *v1, int v2, int v3);
extern int acpserial_buildPackSIF(char *buf, size_t buf_max_len, char sign, const char *v1, int v2, double v3);
extern int acpserial_buildPackSIS(char *buf, size_t buf_max_len, char sign, const char *v1, int v2, const char *v3);
extern int acpserial_buildPackII(char *buf, size_t buf_max_len, char sign, int v1, int v2);
extern int acpserial_buildPackIII(char *buf, size_t buf_max_len, char sign, int v1, int v2, int v3);
extern int acpserial_buildPackIUl(char *buf, size_t buf_max_len, char sign, int v1, unsigned long v2);
extern int acpserial_buildPackIF(char *buf, size_t buf_max_len, char sign, int v1, double v2);
extern int acpserial_buildPackIIF(char *buf, size_t buf_max_len, char sign, int v1, int v2, double v3);
extern int acpserial_buildPackIS(char *buf, size_t buf_max_len, char sign, int v1, const char *v2);
extern int acpserial_buildPackIIS(char *buf, size_t buf_max_len, char sign, int v1, int v2, const char *v3);
extern int acpserial_buildPackIFTS(char *buf, size_t buf_max_len, char sign, int v1, FTS *v2);

extern int acpserial_sendChCmd(int fd, char sign, int cmd, int channel_id);
extern int acpserial_sendChCmdF1(int fd, char sign, int cmd, int channel_id, double v);
extern int acpserial_sendChCmdI1(int fd, char sign, int cmd, int channel_id, int v);
extern int acpserial_sendChCmdStr(int fd, char sign, int cmd, int channel_id, const char *v);

extern int acpserial_extractFTS(const char *buf, size_t len, FTS *v);
extern int acpserial_extractI2(const char *buf, size_t len, int *v1, int *v2);
extern int acpserial_extractSI(const char *buf, size_t len, char *v1, size_t v1_len, int *v2);

#endif 

