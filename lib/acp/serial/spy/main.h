#ifndef ACP_SERIAL_SPY_H
#define ACP_SERIAL_SPY_H

#include "id/list.h"
#include "port/list.h"

/*
 * This is parallel spy for ACP protocol over serial port. 
 * You can open as many serial ports as you need and they will work in parallel.
 * You can add many network object IDs and many commands for each, and all serial
 * ports will be trying to find responces to requests to specified IDs and commands and
 * saving them to RAM. 
 * If port has been closed by OS, this object will try to open it again.
 */
 
#define ACPSY_MAX_PORT_COUNT			16UL
#define ACPSY_MAX_ID_COUNT				256UL
#define ACPSY_MAX_ID_COMMAND_COUNT		64UL

typedef struct {
	AcpsyPortList ports;
	AcpsyIDList ids;
} Acpsy;

extern Acpsy *acpsy_newBegin();

extern int acpsy_createNewPort(Acpsy *self, const char *serial_file_name, int serial_rate, const char *serial_dps);

extern int acpsy_addID(Acpsy *self, int id);

extern int acpsy_addCommandToID(Acpsy *self, int id, int command, struct timespec timeout);

extern int acpsy_getResponse(Acpsy *self, int id, int command, char *out, size_t out_len);

extern void acpsy_free(Acpsy *self);

#endif
