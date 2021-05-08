#include "port_param.h"

int acpspParam_set(AcpSerialPortParam *self, const char *serial_filename, int serial_rate, const char *serial_dps){
	self->filename = NULL;
	if(!serial_checkBaud(serial_rate)){
		return 0;
	}
	if(!serial_checkDps(serial_dps)){
		return 0;
	}
	self->rate = serial_rate;
	memset(self->dps, 0, SERIAL_DPS_STRLEN+1);
	strncpy(self->dps, serial_dps, SERIAL_DPS_STRLEN);
	strcpyma (&self->filename, serial_filename);
	printdo("%s to %s\n", serial_filename, self->filename);
	if(self->filename == NULL){
		return 0;
	}
	return 1;
}

void acpspParam_free(AcpSerialPortParam *self){
	free(self->filename);
}
