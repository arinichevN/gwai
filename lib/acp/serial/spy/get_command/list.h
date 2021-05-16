#ifndef ACP_SERIAL_SPY_GET_COMMAND_LIST_H
#define ACP_SERIAL_SPY_GET_COMMAND_LIST_H

#include "../../../../debug.h"
#include "../../../../dstructure.h"
#include "../../../../app.h"
#include "main.h"

#define ACPSY_GET_COMMAND_LIST_ALLOC_BLOCK_LENGTH 16

DEC_LIST(AcpsyGetCommand)

extern int acpsygcList_begin(AcpsyGetCommandList *self);

//extern int acpsygcLList_add(AcpsyGetCommandLList *self, AcpsyGetCommand *item, size_t items_max_count);

extern int acpsygcList_add(AcpsyGetCommandList *self, int command, struct timespec timeout, size_t items_max_count);

extern void acpsygcList_free(AcpsyGetCommandList *self);

#endif
