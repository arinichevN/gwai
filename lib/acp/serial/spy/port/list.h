#ifndef ACP_SERIAL_SPY_PORT_LIST_H
#define ACP_SERIAL_SPY_PORT_LIST_H

#include "../../../../debug.h"
#include "../../../../dstructure.h"
#include "../../../../app.h"
#include "main.h"

#define ACPSY_PORT_LIST_ALLOC_BLOCK_LENGTH 8

DEC_LIST(AcpsyPort)

extern int acpsypList_begin(AcpsyPortList *self);

//extern int acpsypList_add(AcpsyPortList *self, AcpsyPort *item, size_t items_max_count);

extern int acpsypList_add(AcpsyPortList *self, const char *serial_file_name, int serial_rate, const char *serial_dps, AcpsyIDList *ids, size_t items_max_count);

extern void acpsypList_free(AcpsyPortList *list);

#endif 
