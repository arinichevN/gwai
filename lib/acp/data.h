#ifndef ACP_DATA_H
#define ACP_DATA_H

#define ACP_FLOAT_FORMAT_OUT "%.3f"
#define ACP_FLOAT_FORMAT_IN "%lf"


#include "../dstructure_auto.h"
#include "main.h"


//remote channel
typedef struct {
    int id;
    int channel_id;
    Peer peer;
} RChannel;

DEC_LIST(RChannel)

#endif
