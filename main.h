
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
#define CHANNELS_CONFIG_FILE "" CONF_DIR "channel/items.tsv"
#define CHANNELS_GET_FILE "" CONF_DIR "channel/interface/get.tsv"
#define CHANNELS_SET_FILE "" CONF_DIR "channel/interface/set.tsv"

#define WAIT_RESP_TIMEOUT 1
#define MAX_RETRY 3
#define SLAVE_CMD_MAX_LENGTH 16
#define SLAVE_CMD_MAX_SIZE (SLAVE_CMD_MAX_LENGTH * sizeof(char))
#define SLAVE_TYPE_MAX_LENGTH 16
#define STATE_STR_ONLINE "online"
#define STATE_STR_OFFLINE "offline"

#define STATUS_SUCCESS "SUCCESS"
#define STATUS_FAILURE "FAILURE"

#define TREG_GOOD_STATE 16
#define TREG_BAD_STATE 8

#define CONNECTION_ERROR PUART_CONNECTION_FAILED

#define SLAVE_DATA_BUFFER_LENGTH 128

#define CMD_SLAVE_CHANNEL_GET_DATA_BY_CMD "scgdbc"
#define CMD_SLAVE_CHANNEL_SET_INT "scsi"
#define CMD_SLAVE_CHANNEL_SET_DOUBLE "scsd"

#define SLAVE_TYPE_FTS_STR "fts"
#define SLAVE_TYPE_INT_STR "int"
#define SLAVE_TYPE_DOUBLE_STR "float"

typedef enum {
    SLAVE_TYPE_FTS, 
    SLAVE_TYPE_INT, 
    SLAVE_TYPE_DOUBLE,
    SLAVE_TYPE_UNKNOWN
} SlaveDTypeE;

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

typedef struct {
    double value;
    struct timespec tm;
    int state;
}SFTS;

typedef union {
    SFTS fts;
    int intg;
    double dbl;
} SlaveDType;

typedef struct {
    SlaveDType data;
    int ( *readFunction ) ( int, int, char*, Mutex *, SlaveDType * );
    void ( *sendFunction ) ( int, int, Mutex *, SlaveDType * );
    char cmd[SLAVE_CMD_MAX_LENGTH];
    SlaveDTypeE data_type;
    struct timespec interval;
    
    Mutex mutex;
    Ton tmr;
    int result;
    int state;
} SlaveDataItem;
DEC_LIST(SlaveDataItem)

typedef struct {
    int ( *setFunction ) ( int, int, Mutex *, char *, void * );
    char cmd[SLAVE_CMD_MAX_LENGTH];
    SlaveDTypeE data_type;
} SlaveSetItem;
DEC_LIST(SlaveSetItem)


typedef struct sthread_st SerialThread;

typedef struct {
   int id;
   SlaveDataItemList data_list;
   SlaveSetItemList set_list;
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
	char serial_pattern[LINE_SIZE];
	int serial_rate;
	char serial_config[LINE_SIZE];
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




