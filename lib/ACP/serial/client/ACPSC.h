#ifndef ACP_SERIAL_CLIENT_H
#define ACP_SERIAL_CLIENT_H

#include "ACPSCIDLListm.h"
#include "ACPSCPortLListm.h"
#include "ACPSCIDMapper.h"

/*
 * This is parallel client with routing capabilities for ACP protocol over serial port. 
 * You can open as many serial ports as you need and they will work in parallel.
 * You can add many remote IDs, and this object will try to find them on serial ports. And 
 * when you try to send data to remote ID, this object will use the serial port to which
 * remote ID is mapped (routing capabilities).
 * If connection to remote ID is lost, this object will try to find it.
 * If port has been closed by OS, this object will try to open it again.
 */
 
#define ACPSC_MAX_PORT_COUNT			16UL
#define ACPSC_MAX_ID_COUNT				256UL

typedef struct {
	ACPSCPortLListm ports;
	ACPSCIDLListm ids;
	ACPSCIDMapper *id_mapper;
} ACPSC;

extern ACPSC *acpsc_newBegin();

extern int acpsc_createNewPort(ACPSC *self, const char *serial_file_name, int serial_rate, const char *serial_config) ;

extern int acpsc_addRemoteID(ACPSC *self, int id);

extern void acpsc_free(ACPSC *self);

extern int acpsc_getFromRemoteID(ACPSC *self, int remote_id, const char *request_str, char *response, size_t response_length);

extern int acpsc_getBroadcast(ACPSC *self, const char *request_str, char *response, size_t response_length);

extern int acpsc_sendRequestToRemoteID(ACPSC *self, int remote_id, const char *request_str);

extern int acpsc_sendRequestBroadcast(ACPSC *self, const char *request_str);

#endif
