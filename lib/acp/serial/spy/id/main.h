#ifndef ACP_SERIAL_SPY_ID_H
#define ACP_SERIAL_SPY_ID_H

#include "../../../../debug.h"
#include "../../../../app.h"
#include "../get_command/list.h"

typedef struct acpsyid_st AcpsyID;
struct acpsyid_st {
	int value;
	AcpsyGetCommandList gcommands;
	AcpsyID *next;
};

//extern AcpsyID *acpsyid_newBegin(int id);

extern int acpsyid_begin(AcpsyID *self, int id);

extern void acpsyid_free(AcpsyID *self);

extern int acpsyid_addCommand(AcpsyID *self, int command_id, struct timespec timeout, size_t items_max_count);

extern AcpsyGetCommand *acpsyid_getCommandById(AcpsyID *self, int command_id);

#endif
