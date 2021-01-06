#ifndef ACP_SERIAL_CLIENT_ID_H
#define ACP_SERIAL_CLIENT_ID_H

#include "../../../debug.h"
#include "../../../app.h"


typedef enum {
	ACPSCID_NOT_FOUND,
	ACPSCID_FOUND,
	ACPSCID_PORT_DISCONNECTED
} ACPSCIDState;

typedef struct acpscid_st ACPSCID;
struct acpscid_st {
	int value;
	void *owner;
	ACPSCIDState state;
	Mutex mutex;
	ACPSCID *next;
};

extern ACPSCID *acpscid_newBegin(int id);

extern void acpscid_free(ACPSCID *self);

extern void acpscid_lock(ACPSCID *self);

extern void acpscid_unlock(ACPSCID *self);

extern int acpscid_isOwned(ACPSCID *self);

extern void acpscid_setOwner(ACPSCID *self, void *owner);

extern const char *acpscid_getStateStr(ACPSCID *self);

#endif
