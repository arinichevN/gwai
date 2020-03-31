#ifndef MODEL_SLAVE_GET_COMMAND_H
#define MODEL_SLAVE_GET_COMMAND_H

#include "../lib/acp/tcp/main.h"
#include "../lib/app.h"
#include "../lib/tsv.h"

#define SLAVE_CMD_MAX_LENGTH ACP_CMD_MAX_LENGTH
#define SLAVE_CMD_MAX_SIZE (SLAVE_CMD_MAX_LENGTH * sizeof(char))

typedef struct {
    char data[ACP_BUF_MAX_LENGTH];
    char name[ACP_CMD_MAX_LENGTH];
    Mutex mutex;
    int result;
} SlaveGetCommand;
DEC_LIST(SlaveGetCommand)

extern void sgc_free(SlaveGetCommand *item);
extern int sgcList_init(SlaveGetCommandList *list, const char *dir, const char *file_name, const char *file_type);
extern void sgc_sendDataToClient (SlaveGetCommand *item, int tcp_fd );
extern void sgc_setData(SlaveGetCommand *item, const char *data, size_t data_len, int result);

#endif 
