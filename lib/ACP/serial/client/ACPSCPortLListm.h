#ifndef ACP_SERIAL_CLIENT_PORT_LLIST_H
#define ACP_SERIAL_CLIENT_PORT_LLIST_H

#include "../../../debug.h"
#include "../../../dstructure.h"
#include "../../../app.h"
#include "ACPSCPort.h"

DEC_LLISTM(ACPSCPort)

extern int acpscpLList_begin(ACPSCPortLListm *self);

extern int acpscpLList_add(ACPSCPortLListm *self, ACPSCPort *item, size_t items_max_count);

extern void acpscpLList_free(ACPSCPortLListm *list);

#endif 
