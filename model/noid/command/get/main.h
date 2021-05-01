#ifndef MODEL_NOID_COMMAND_GET_H
#define MODEL_NOID_COMMAND_GET_H

#include "../../../../lib/acp/tcp/main.h"
#include "../../../../lib/app.h"
#include "../../../../lib/tsv.h"

#define NOID_CMD_MAX_LENGTH ACP_CMD_MAX_LENGTH
#define NOID_CMD_MAX_SIZE (NOID_CMD_MAX_LENGTH * sizeof(char))

typedef struct {
	int id;
	char data[ACP_BUF_MAX_LENGTH];
	Mutex mutex;
	int result;
} NoidGetCommand;

DEC_LIST(NoidGetCommand)

extern void ngc_free(NoidGetCommand *item);
extern void ngc_sendDataToClient (NoidGetCommand *item, int tcp_fd );
extern void ngc_setData(NoidGetCommand *item, const char *data, size_t data_len, int result);
extern int ngcList_init(NoidGetCommandList *list, const char *dir, const char *file_name, const char *file_type);

#endif 
