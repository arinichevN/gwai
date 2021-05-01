#ifndef ACP_SERIAL_PORT_PARAM_H
#define ACP_SERIAL_PORT_PARAM_H

#include "../../serial.h"
#include "../../app.h"

typedef struct {
	char *filename;
	char config[SERIAL_CONFIG_STRLEN + 1];
	int rate;
} AcpSerialPortParam;

extern int acpspParam_set(AcpSerialPortParam *self, const char *serial_filename, int serial_rate, const char *serial_config);

extern void acpspParam_free(AcpSerialPortParam *self);

#endif 
