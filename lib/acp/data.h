#ifndef LIBPAS_ACP_DATA_H
#define LIBPAS_ACP_DATA_H

#define ACP_FLOAT_FORMAT_OUT "%.3f"
#define ACP_FLOAT_FORMAT_IN "%lf"


#include "../dstructure_auto.h"
#include "main.h"


typedef int I1;
DEC_LIST(I1)

typedef struct {
    int p0;
    int p1;
} I2;

DEC_LIST(I2)

typedef struct {
    int p0;
    int p1;
    int p2;
} I3;
DEC_LIST(I3)

typedef double F1;
DEC_LIST(F1)

typedef double D1;
DEC_LIST(D1)

typedef struct {
    int p0;
    double p1;
} I1F1;

DEC_LIST(I1F1)

typedef struct {
    int p0;
    uint32_t p1;
} I1U321;
DEC_LIST(I1U321)

typedef char S1;

DEC_LIST(S1)


typedef struct {
    int id;
    double value;
    struct timespec tm;
    int state;
} FTS;

DEC_LIST(FTS)

typedef struct {
    int id;
    int value;
    struct timespec tm;
    int state;
} ITS;

DEC_LIST(ITS)

//remote channel
typedef struct {
    int id;
    int channel_id;
    Peer peer;
} RChannel;

DEC_LIST(RChannel)


#endif
