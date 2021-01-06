#ifndef ACP_SERIAL_CLIENT_ID_MAPPER_H
#define ACP_SERIAL_CLIENT_ID_MAPPER_H

#include "../../ACP.h"
#include "ACPSCIDLListm.h"
#include "ACPSCPortLListm.h"

#define ACPSCIDM_CYCLE_DURATION_OFF					(struct timespec) {0, 500000000UL}
#define ACPSCIDM_CYCLE_SEARCH_UNCONNECTED			(struct timespec) {3, 500000000UL}
#define ACPSCIDM_CYCLE_SEARCH_ALL					(struct timespec) {0, 0}

typedef struct acpscidmapper_st ACPSCIDMapper;
struct acpscidmapper_st {
	ACPSCPortLListm *ports;
	ACPSCIDLListm *ids;
	Thread thread;
	Mutex mutex;
	struct timespec cycle_duration;
	void (*control)(ACPSCIDMapper *);
};

extern ACPSCIDMapper *acpscim_newBegin(ACPSCPortLListm *ports, ACPSCIDLListm *ids);

extern void acpscim_free(ACPSCIDMapper *self);

extern void acpscim_IDLost(ACPSCIDMapper *self, ACPSCID *id);

extern void acpscim_connectToPort(ACPSCIDMapper *self, ACPSCPort *port);

#endif
