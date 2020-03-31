#ifndef MODEL_SLAVE_BGET_COMMAND_H
#define MODEL_SLAVE_BGET_COMMAND_H

#include "SerialThread.h"

typedef struct {
    char data[ACP_BUF_MAX_LENGTH];
    char name[ACP_CMD_MAX_LENGTH];
    SerialThread *stread_success;
    Mutex mutex;
    int result;
} SlaveBGetCommand;
DEC_LIST(SlaveBGetCommand)

extern SlaveBGetCommand *sbgc_getByCmd(const char *cmd, SlaveBGetCommandList *list);
extern int sbgc_initList(SlaveBGetCommandList *list, const char *path );

#endif
