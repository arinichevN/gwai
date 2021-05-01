#ifndef ACP_TCP_SERVER_CONNECTION_LIST_H
#define ACP_TCP_SERVER_CONNECTION_LIST_H

#include "../../../debug.h"
#include "../../../dstructure_auto.h"
#include "connection.h"

DEC_LLIST(AcptsConnection)

extern void acptsconnList_free(AcptsConnectionLList *list);

extern int acptsconnList_push(AcptsConnectionLList *list, AcptsConnection *item);

#endif
