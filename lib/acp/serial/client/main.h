#ifndef ACP_SERIAL_CLIENT_H
#define ACP_SERIAL_CLIENT_H

#include "id/list.h"
#include "port/list.h"

/*
 * This is parallel client with routing capabilities for ACP protocol over serial port. 
 * You can open as many serial ports as you need and they will work in parallel.
 * You can add many network object IDs, and this object will try to find them on serial ports. And 
 * when you try to send data to remote ID, this object will use the serial port to which
 * remote ID is mapped (routing capabilities).
 * If port has been closed by OS, this object will try to open it again.
 */
 
#define ACPSC_MAX_PORT_COUNT			16UL
#define ACPSC_MAX_ID_COUNT				256UL

typedef struct {
	AcpscPortLListm ports;
	AcpscIDLListm ids;
} Acpsc;

extern Acpsc *acpsc_newBegin();

extern int acpsc_createNewPort(Acpsc *self, const char *serial_file_name, int serial_rate, const char *serial_dps) ;

extern int acpsc_addRemoteID(Acpsc *self, int id);

extern void acpsc_free(Acpsc *self);

extern int acpsc_getFromRemoteID(Acpsc *self, int remote_id, const char *request_str, char *response, size_t response_length);

extern int acpsc_getBroadcast(Acpsc *self, const char *request_str, char *response, size_t response_length);

extern int acpsc_sendRequestToRemoteID(Acpsc *self, int remote_id, const char *request_str);

extern int acpsc_sendRequestBroadcast(Acpsc *self, const char *request_str);

#endif
