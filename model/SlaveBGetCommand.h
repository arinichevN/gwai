#ifndef MODEL_SLAVE_BGET_COMMAND_H
#define MODEL_SLAVE_BGET_COMMAND_H

#include "SerialThread.h"

typedef struct {
	int id;
    char data[ACP_BUF_MAX_LENGTH];
    SerialThread *stread_success;
    Mutex mutex;
    int result;
} SlaveBGetCommand;
DEC_LIST(SlaveBGetCommand)

extern SlaveBGetCommand *sbgc_getByCmd(int cmd, SlaveBGetCommandList *list);
extern int sbgc_initList(SlaveBGetCommandList *list, const char *path );

#endif
