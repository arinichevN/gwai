#ifndef ACP_SERIAL_CLIENT_ID_LLIST_H
#define ACP_SERIAL_CLIENT_ID_LLIST_H

#include "../../../debug.h"
#include "../../../dstructure.h"
#include "../../../app.h"
#include "ACPSCID.h"

DEC_LLISTM(ACPSCID)

extern int acpscidLList_begin(ACPSCIDLListm *self);

extern int acpscidLList_add(ACPSCIDLListm *self, ACPSCID *item, size_t items_max_count);

extern void acpscidLList_free(ACPSCIDLListm *list);

#endif 
