#ifndef ACP_SERIAL_SERVER_H
#define ACP_SERIAL_SERVER_H

#include "ACPSSCommandNode.h"
#include "ACPSSPort.h"
#include "../../../dstructure.h"
#include "../../../debug.h"
#include "../../../app.h"

#define ACPSS_MAX_PORT_COUNT			16UL

DEC_LLISTM(ACPSSPort)

typedef struct {
	ACPSSPortLListm ports;
} ACPSS;

extern ACPSS *acpss_new();

extern int acpss_createNewPort(ACPSS *self, const char *serial_file_name, int serial_rate, const char *serial_config, ACPSSCommandNode *acnodes, size_t acnodes_count);

extern void acpss_free(ACPSS *self);

#endif
