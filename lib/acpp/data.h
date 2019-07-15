#ifndef LIBPAS_ACPP_DATA_H
#define LIBPAS_ACPP_DATA_H

#define ACPP_FLOAT_FORMAT_OUT "%.3f"
#define ACPP_FLOAT_FORMAT_IN "%lf"


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
    int p0;
    char p1[LINE_SIZE];
} I1S1;

DEC_LIST(I1S1)

typedef struct {
    char p0[LINE_SIZE];
    char p1[LINE_SIZE];
} S2;

DEC_LIST(S2)

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

extern int acpp_cpPeer ( Peer *dest, const Peer * src ) ;
extern int acpp_initPeer ( Peer* item ) ;
extern int acpp_initPeerList ( PeerList* list ) ;
extern int acpp_initRChannel ( RChannel* item ) ;
extern int acpp_cpRChannel ( RChannel *dest, const RChannel *src ) ;
extern int acpp_getRChannelFromList ( RChannel *dest , const RChannelList *list, int id ) ;

extern int acpp_buftodata ( char **v ) ;
extern void acpp_dumpBuf ( const char *buf, size_t buf_size );

extern int acpp_getI ( char *s, int *data ) ;
extern int acpp_getF (char *s, double *data ) ;
extern void acpp_getI1List (char *s, void *data ) ;
extern void acpp_getI2List (char *s, void *data ) ;
extern void acpp_getI3List (char *s, void *data ) ;
extern void acpp_getF1List (char *s, void *data );
extern void acpp_getI1F1List (char *s, void *data ) ;
extern void acpp_getI1U321List (char *s, void *data ) ;
extern void acpp_getS1List (char *s, void *data ) ;
extern void acpp_getI1S1List (char *s, void *data ) ;
extern void acpp_getFTSList (char *s, void *data ) ;
extern void acpp_getITSList (char *s, void *data ) ;
extern void acpp_getS2List (char *s, void *data ) ;


extern void acpp_readI1List ( void *data, int fd ) ;
extern void acpp_readI1F1List ( void *data, int fd ) ;
extern void acpp_readI2List ( void *data, int fd ) ;
extern void acpp_readI1S1List ( void *data, int fd );

extern int acpp_setI1List ( char *s, void *data ) ;
extern int acpp_setI1F1List ( char *s, void *data );
//*******************************************************************************************************************************
extern void acpp_sendFTSList(FTSList *list, int fd);
//*******************************************************************************************************************************
extern int acpp_sendRequest ( char *cmd, void *data, int ( *data_set_fun ) ( char *, void * ), int fd );

extern int acpp_readData ( void *data, void ( *data_get_fun ) ( char *, void * ), int fd ) ;



//**************************************************************************************************************************+++++++
extern int acpp_peerSendCmd ( const char *cmd, Peer *peer ) ;
extern int acpp_peerGetFTSList ( FTSList *output, Peer *peer, I1List *input ) ;

extern int acpp_peerSendI1ListCmd   (char *cmd, I1List *input,   Peer *peer );
extern int acpp_peerSendI1F1ListCmd (char *cmd, I1F1List *input, Peer *peer );

extern int acpp_FTSCat ( int id, double value, struct timespec tm, int state, char *buf ) ;
extern int acpp_IDCat ( int id, double value, char *buf );
extern int acpp_I2Cat ( int p0, int p1, char *buf );
extern int acpp_I3Cat ( int p0, int p1, int p2, char *buf );
extern int acpp_I4Cat ( int p0, int p1, int p2, int p3, char *buf );
extern int acpp_I5Cat ( int p0, int p1, int p2, int p3, int p4, char *buf );
#endif
