#ifndef ACP_SERIAL_CLIENT_PORT_LLIST_H
#define ACP_SERIAL_CLIENT_PORT_LLIST_H

#include "../../../../debug.h"
#include "../../../../dstructure.h"
#include "../../../../app.h"
#include "main.h"

DEC_LLISTM(AcpscPort)

extern int acpscpLList_begin(AcpscPortLListm *self);

extern int acpscpLList_add(AcpscPortLListm *self, AcpscPort *item, size_t items_max_count);

extern void acpscpLList_free(AcpscPortLListm *list);

#endif 
