#ifndef ACP_SERIAL_SERVER_COMMAND_NODE_H
#define ACP_SERIAL_SERVER_COMMAND_NODE_H

typedef struct {
	int command;
	int (*func) (char *buf);
} ACPSSCommandNode;

#endif 
