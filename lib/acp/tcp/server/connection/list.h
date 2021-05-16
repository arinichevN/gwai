#ifndef ACP_TCP_SERVER_CONNECTION_LIST_H
#define ACP_TCP_SERVER_CONNECTION_LIST_H

#include "../../../../debug.h"
#include "../../../../dstructure.h"
#include "main.h"

DEC_LLIST(AcptsConnection)


extern int acptsconnList_push(AcptsConnectionLList *list, AcptsConnection *item);

extern void acptsconnList_terminate(AcptsConnectionLList *list);

extern void acptsconnList_free(AcptsConnectionLList *list);

#endif
