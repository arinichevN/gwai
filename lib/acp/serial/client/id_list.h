#ifndef ACP_SERIAL_CLIENT_ID_LLIST_H
#define ACP_SERIAL_CLIENT_ID_LLIST_H

#include "../../../debug.h"
#include "../../../dstructure.h"
#include "../../../app.h"
#include "id.h"

DEC_LLISTM(AcpscID)

extern int acpscidLList_begin(AcpscIDLListm *self);

extern int acpscidLList_add(AcpscIDLListm *self, AcpscID *item, size_t items_max_count);

extern void acpscidLList_free(AcpscIDLListm *list);

#endif 
