#ifndef MODEL_CHANNEL_LIST_H
#define MODEL_CHANNEL_LIST_H

#include "../../lib/debug.h"
#include "../../lib/dstructure.h"
#include "main.h"

DEC_LIST(Channel)

extern void channelList_free (ChannelList *list);

#endif 
