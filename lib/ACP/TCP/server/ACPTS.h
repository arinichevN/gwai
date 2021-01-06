#ifndef ACP_TCP_SERVER_H
#define ACP_TCP_SERVER_H

#include "../../../timef.h"
#include "../../../dstructure_auto.h"
#include "../../../app.h"
#include "../ACPTCP.h"
#include "ACPTSConnection.h"
#include "ACPTSConnectionLList.h"
#include "ACPTSServeFunction.h"

typedef int(*ACPTSServeFunction)(int, const char *);

typedef struct {
	ACPTSConnectionLList connections;
	int fd;
	size_t conn_num_max;
	int (*serve_function) (int, const char *);
	Thread thread;
} ACPTS;

extern void acpts_free(ACPTS *self);

extern ACPTS *acpts_newBegin(int port, size_t conn_num_max, ACPTSServeFunction serve_function);

#endif
