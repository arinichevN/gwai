#include "ACPSerialPortParam.h"

int acpspParam_set(ACPSerialPortParam *self, const char *serial_filename, int serial_rate, const char *serial_config){
	self->filename = NULL;
	if(!serial_checkBaud(serial_rate)){
		return 0;
	}
	if(!serial_checkConfig(serial_config)){
		return 0;
	}
	self->rate = serial_rate;
	memset(self->config, 0, SERIAL_CONFIG_STRLEN+1);
	strncpy(self->config, serial_config, SERIAL_CONFIG_STRLEN);
	strcpyma (&self->filename, serial_filename);
	printdo("%s to %s\n", serial_filename, self->filename);
	if(self->filename == NULL){
		return 0;
	}
	return 1;
}

void acpspParam_free(ACPSerialPortParam *self){
	free(self->filename);
}
