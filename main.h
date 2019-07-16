
#ifndef GWAI_H
#define GWAI_H

#include <dirent.h>
#include "lib/util.h"
#include "lib/app.h"
#include "lib/timef.h"
#include "lib/tsv.h"
#include "lib/puart.h"


#include "lib/acpp/main.h"
#include "lib/acpp/app.h"
#include "lib/acpp/server/common.h"
#include "lib/acpp/server/parallel.h"
#include "lib/acpp/cmd/main.h"


#define APP_NAME gwai
#define APP_NAME_STR TOSTRING(APP_NAME)


#ifdef MODE_FULL
#define CONF_DIR "/etc/controller/" APP_NAME_STR "/"
#endif
#ifndef MODE_FULL
#define CONF_DIR "./config/"
#endif
#define CONFIG_FILE "" CONF_DIR "app.tsv"
#define CHANNELS_CONFIG_FILE "" CONF_DIR "channels.tsv"

#define WAIT_RESP_TIMEOUT 1
#define MAX_RETRY 3
#define SOUND_QUEUE_LENGTH 8
#define STATE_STR_ONLINE "online"
#define STATE_STR_OFFLINE "offline"


#define STATUS_SUCCESS "SUCCESS"
#define STATUS_FAILURE "FAILURE"


#define TREG_GOOD_STATE 16
#define TREG_BAD_STATE 8

#define CONNECTION_ERROR PUART_CONNECTION_FAILED

typedef enum {
   OFF,
   INIT,
   RUN,
   DO,
   TERMINATED,
   SEARCH_NEED,
   SEARCH_PATH,
   FIND_CHANNELS,
   SLAVE_GET,
   SLAVE_RESET,
   SLAVE_COOP,
   BUSY,
   IDLE,
   WAIT,
   OPENED,
   CLOSED,
   OPEN,
   CLOSE,
   UNDEFINED,
   DISABLE,
   FAILURE
} ProgState;

typedef enum {
	FAILURE_NO,
	FAILURE_CONNECTION,
	FAILURE_SAVE,
	FAILURE_GET,
	FAILURE_RESET,
	FAILURE_RACK_GET_HIVE_COUNT_NO,
	FAILURE_RACK_MEM,
	FAILURE_HIVE_COUNT,
} Fail;
//data from slave
typedef struct {
	double input;
	int input_state;
	int input_stm;
	struct timespec input_tm;
	
	struct timespec interval;
	
	struct timespec tm;
	Ton tmr;
	int state;
	Mutex mutex;
} RunData;

typedef struct sthread_st SerialThread;

typedef struct {
   int id;
   RunData run;
   SerialThread *thread;
   int max_retry;
   int retry;
   int state;
   Mutex mutex;
} Channel;
DEC_LIST(Channel)

typedef struct channelptr_st ChannelPtr;
struct channelptr_st{
	Channel *item;
	struct channelptr_st *next;
};
DEC_LIST(ChannelPtr)
DEC_LLIST(ChannelPtr)

//SerialThread searches for channels on it's port, adds found channels to it's channel list and polls this channels  
struct sthread_st{
	int id;
	char serial_path[LINE_SIZE];
	int serial_rate;
	int max_retry;
	int retry;
	int state;
	ChannelPtrList channelptr_list;//links to all available channels, this list is the same for each thread, we use this links in channelptr_llist
	ChannelPtrLList channelptr_llist;//channels beeing controlled by this thread
	Mutex rmutex;
	int chpl_ind;//current channelptr_list index for use in searchNAddUnconnectedChannel()
	int fd;
	pthread_t thread;
	struct timespec cycle_duration;
	Mutex mutex;
	struct sthread_st *next;
};
DEC_LLIST(SerialThread)

//SerialThreadStarter creates SerialThread for each detected serial port
typedef struct {
	DIR *dfd;
	char *dir;
	char *serial_pattern;
	int serial_rate;
	char *serial_config;
	struct timespec thread_cd;
	
	int state;
	pthread_t thread;
} SerialThreadStarter;

extern int readSettings();

extern int initData();

extern int initApp();

extern void serverRun ( int *state, int init_state );

extern void freeData();

extern void freeApp();

extern void exit_nicely();

#endif




