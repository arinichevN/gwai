#ifndef ACP_SERIAL_SERVER_H
#define ACP_SERIAL_SERVER_H

#include "../../../dstructure.h"
#include "../../../debug.h"
#include "../../../app.h"
#include "command_node.h"
#include "port.h"

#define ACPSS_MAX_PORT_COUNT			16UL

DEC_LLISTM(AcpssPort)

typedef struct {
	AcpssPortLListm ports;
} Acpss;

extern Acpss *acpss_new();

extern int acpss_createNewPort(Acpss *self, const char *serial_file_name, int serial_rate, const char *serial_config, AcpssCommandNode *acnodes, size_t acnodes_count);

extern void acpss_free(Acpss *self);

#endif
