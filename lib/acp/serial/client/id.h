#ifndef ACP_SERIAL_CLIENT_ID_H
#define ACP_SERIAL_CLIENT_ID_H

#include "../../../debug.h"
#include "../../../app.h"


typedef enum {
	ACPSCID_NOT_FOUND,
	ACPSCID_FOUND,
	ACPSCID_PORT_DISCONNECTED
} AcpscIDState;

typedef struct acpscid_st AcpscID;
struct acpscid_st {
	int value;
	void *owner;
	AcpscIDState state;
	Mutex mutex;
	AcpscID *next;
};

extern AcpscID *acpscid_newBegin(int id);

extern void acpscid_free(AcpscID *self);

extern void acpscid_lock(AcpscID *self);

extern void acpscid_unlock(AcpscID *self);

extern int acpscid_isOnline(AcpscID *self);

extern int acpscid_setOwner(AcpscID *self, void *owner);

extern int acpscid_needSearchOnPort(AcpscID *self, void *owner);

extern const char *acpscid_getStateStr(AcpscID *self);

#endif
