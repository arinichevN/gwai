#ifndef ACP_SERIAL_SPY_ID_LIST_H
#define ACP_SERIAL_SPY_ID_LIST_H

#include "../../../../debug.h"
#include "../../../../dstructure.h"
#include "../../../../app.h"
#include "main.h"

#define ACPSY_ID_LIST_ALLOC_BLOCK_LENGTH 16

DEC_LIST(AcpsyID)

extern int acpsyidList_begin(AcpsyIDList *self);

//extern int acpsyidLList_add(AcpsyIDLListm *self, AcpsyID *item, size_t items_max_count);

extern int acpsyidList_add(AcpsyIDList *self, int id, size_t items_max_count);

extern void acpsyidList_free(AcpsyIDList *self);

#endif 
