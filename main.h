
#ifndef GWST_H
#define GWST_H

#include "lib/util.h"
#include "lib/app.h"
#include "lib/timef.h"
#include "lib/tsv.h"

#include "lib/acp/main.h"
#include "lib/acp/app.h"
#include "lib/acp/cmd/main.h"
#include "lib/acp/tcp/main.h"
#include "lib/acp/tcp/server/common.h"
#include "lib/acp/tcp/server/parallel.h"
#include "lib/acp/serial/main.h"

#include "model/SlaveGetCommand.h"
#include "model/SlaveSetCommand.h"
#include "model/SlaveIntervalGetCommand.h"
#include "model/SlaveBGetCommand.h"
#include "model/Channel.h"
#include "model/Channel.h"
#include "model/SerialThread.h"
#include "model/SerialThreadStarter.h"

#define APP_NAME gwst
#define APP_NAME_STR TOSTRING(APP_NAME)

#ifdef MODE_FULL
#define CONF_DIR "/etc/controller/" APP_NAME_STR "/config/"
#endif
#ifndef MODE_FULL
#define CONF_DIR "./config/"
#endif
#define CONF_FILE_TYPE ".tsv"
#define CONFIG_FILE "" CONF_DIR "app.tsv"
#define BROADCAST_CONFIG_DIR "./config/broadcast/"
#define BROADCAST_GET_CONFIG_FILE "" BROADCAST_CONFIG_DIR "get.tsv"
#define BROADCAST_SET_CONFIG_FILE "set"
#define CHANNELS_CONFIG_FILE "" CONF_DIR "channel/items.tsv"
#define CHANNELS_IGET_DIR "" CONF_DIR "channel/interface/get/interval/"
#define CHANNELS_GET_DIR "" CONF_DIR "channel/interface/get/simple/"
#define CHANNELS_TGET_DIR "" CONF_DIR "channel/interface/get/text/"
#define CHANNELS_SET_DIR "" CONF_DIR "channel/interface/set/"

#define SRV_SLEEP_BEFORE_READ_SLAVE NANOSLEEP(0,100000000);

//#define CONFIG_DELIMITER_COLUMN '*'


//#define WAIT_RESP_TIMEOUT 1
//#define MAX_RETRY 3
//#define SLAVE_CMD_MAX_LENGTH ACP_CMD_MAX_LENGTH
//#define SLAVE_CMD_MAX_SIZE (SLAVE_CMD_MAX_LENGTH * sizeof(char))

//#define SLEEP_BEFORE_READ_SLAVE NANOSLEEP(0,100000000);

//#define CMD_CHANNEL_EXISTS ACP_CMD_CHANNEL_EXISTS

extern int readSettings();

extern int initData();

extern int initApp();

extern void serverRun ( int *state, int init_state );

extern void freeData();

extern void freeApp();

extern void exit_nicely();

#endif




