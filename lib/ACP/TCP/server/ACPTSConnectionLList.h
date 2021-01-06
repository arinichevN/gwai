#ifndef ACP_TCP_SERVER_CONNECTION_LIST_H
#define ACP_TCP_SERVER_CONNECTION_LIST_H

#include "../../../debug.h"
#include "../../../dstructure_auto.h"
#include "ACPTSConnection.h"

DEC_LLIST(ACPTSConnection)

extern void acptsconnList_free(ACPTSConnectionLList *list);

extern int acptsconnList_push(ACPTSConnectionLList *list, ACPTSConnection *item);

#endif
