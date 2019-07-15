#ifndef LIBPAS_ACPP_MAIN_H
#define LIBPAS_ACPP_MAIN_H

#include <netinet/in.h>

#include "../dstructure_auto.h"
#define ACPP_RETRY_NUM 12
#define ACPP_BUFFER_SIZE 1024
#define ACPP_PACK_SIZE 8

#define ACPP_DELIMITER_COLUMN  '\t'
#define ACPP_DELIMITER_COLUMN_STR  "\t"
#define ACPP_DELIMITER_ROW  '\n'
#define ACPP_DELIMITER_ROW_STR  "\n"
#define ACPP_DELIMITER_BLOCK  '\r'
#define ACPP_DELIMITER_END  '\f'
#define ACPP_DELIMITER_END_STR  "\f"
#define ACPP_DELIMITER_BLOCK_STR "\r"

#define ACPP_RETURN_SUCCESS  1
#define ACPP_RETURN_FAILURE  0
#define ACPP_RETURN_WAIT_SEND  2
#define ACPP_RETURN_WAIT_RECIEVE  3

#define ACPP_PEER_ID_LENGTH  16
#define ACPP_PEER_ADDR_STR_LENGTH  48
#define ACPP_CMD_MAX_LENGTH 16
#define ACPP_ROW_MAX_LENGTH 256

#define ACPP_DEF_BUFFER(b) char b[ACPP_BUFFER_SIZE];memset(b, 0, sizeof b);

typedef struct {
    char id[ACPP_PEER_ID_LENGTH];
    char addr_str[ACPP_PEER_ADDR_STR_LENGTH];
    int iid;
    int port;
    int fd;
    int fd_conn;
    struct sockaddr_in addr;
    socklen_t addr_size;
} Peer;
DEC_LIST(Peer)



extern int acpp_initServer(int *fd, int port) ;
extern int acpp_initClient(Peer *peer, __time_t tmo) ;
extern int acpp_makeClientAddr(struct sockaddr_in *addr, const char *addr_str, int port) ;

extern int acpp_getCmdLength ( char *buf );

extern void acpp_dumpBuf ( const char *buf, size_t buf_size );

extern int acpp_bufCountDataRows ( const char *buf );

extern int acpp_cmdcmp ( char *buf, char * cmd ) ;

extern int acpp_setCmd ( char * buf, const char *cmd ) ;

extern int acpp_setEnd ( char * buf );

extern int acpp_strCat ( char *buf, const char *str ) ;

extern int acpp_read ( char *buf, int fd );

extern size_t acpp_readCmd ( char *buf, size_t length, int fd );

extern size_t acpp_readRow ( char *buf, size_t length, int fd );

extern int acpp_send ( const char *s, int fd ) ;


#endif 

