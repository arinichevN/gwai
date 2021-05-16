#ifndef ACP_SERIAL_SPY_GET_COMMAND_H
#define ACP_SERIAL_SPY_GET_COMMAND_H

#include "../../../../debug.h"
#include "../../../../app.h"
#include "../../../../timef.h"
#include "../../../main.h"

//#define SLAVE_CMD_MAX_LENGTH ACP_CMD_MAX_LENGTH
//#define SLAVE_CMD_MAX_SIZE (SLAVE_CMD_MAX_LENGTH * sizeof(char))

typedef struct acpsygetcommand_st AcpsyGetCommand;
struct acpsygetcommand_st {
	char data[ACP_BUF_MAX_LENGTH];
	int value;
	Mutex mutex;
	struct timespec timeout;
	struct timespec write_time;
};

//extern GetCommand *acpsygc_newBegin(int command);
extern int acpsygc_begin(AcpsyGetCommand *self, int command, struct timespec timeout);
extern void acpsygc_free(AcpsyGetCommand *self);
extern int acpsygc_getData(AcpsyGetCommand *self, int tcp_fd, char *buf, size_t data_len);
extern void acpsygc_setData(AcpsyGetCommand *self, const char *data, size_t data_len);

#endif
