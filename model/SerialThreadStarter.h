#ifndef MODEL_SERIAL_THREAD_STARTER_H
#define MODEL_SERIAL_THREAD_STARTER_H

#include <dirent.h>
#include "../lib/util.h"
#include "../lib/app.h"
#include "../lib/timef.h"
#include "../lib/serial.h"
#include "SerialThread.h"

//SerialThreadStarter creates SerialThread for each detected serial port
typedef struct sts_st{
	DIR *dfd;
	char *dir;
	char serial_pattern[LINE_SIZE];
	int serial_rate;
	char serial_config[LINE_SIZE];
	struct timespec thread_cd;
	int max_retry;
	
	void (*control)(struct sts_st *item);
	pthread_t thread;
} SerialThreadStarter;


extern int sts_init(SerialThreadStarter *item, int max_retry);
extern void sts_free(SerialThreadStarter *item);
extern const char *sts_getStateStr(SerialThreadStarter *item);
#define sts_control(item) (item)->control(item)

#endif
