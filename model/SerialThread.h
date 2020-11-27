#ifndef MODEL_SERIAL_THREAD_H
#define MODEL_SERIAL_THREAD_H

#include "../lib/acp/cmd/main.h"
#include "../lib/acp/serial/main.h"
#include "../lib/util.h"
#include "../lib/app.h"
#include "../lib/timef.h"
#include "../lib/serial.h"
#include "Channel.h"

#define ST_SLEEP_BEFORE_READ_SLAVE NANOSLEEP(0,100000000);

//SerialThread searches for channels on it's port, adds found channels to it's channel list and polls this channels  
typedef struct sthread_st SerialThread;
struct sthread_st {
	int id;
	char serial_path[LINE_SIZE];
	int serial_rate;
	int max_retry;
	int retry;
	void (*control)(SerialThread *);
	ChannelPtrList channelptr_list;//links to all available channels, this list is the same for each thread, we use this links in channelptr_llist
	ChannelPtrLList channelptr_llist;//channels beeing controlled by this thread
	Mutex rmutex;
	int chpl_ind;//current channelptr_list index for use in searchNAddUnconnectedChannel()
	int fd;
	pthread_t thread;
	struct timespec cycle_duration;
	Mutex mutex;
	SerialThread *next;
};

DEC_LLIST(SerialThread)

extern void stList_free ( SerialThreadLList *list ) ;
extern void st_setParam(SerialThread *item, char *serial_path, int fd, int serial_rate, struct timespec cycle_duration);
extern void st_start(SerialThread *item);
extern int st_addNewToList(SerialThreadLList *list, Mutex *list_mutex, ChannelList *rlist, int id, int max_retry, char *serial_path, int fd, int serial_rate, struct timespec cycle_duration);
extern SerialThread *stList_getIdleThread(SerialThreadLList *list);
extern const char *st_getStateStr(SerialThread *item);
extern int st_isTerminated(SerialThread *thread);
#define st_control(item) (item)->control(item)

#endif 
