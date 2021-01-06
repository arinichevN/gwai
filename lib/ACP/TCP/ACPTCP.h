#ifndef LIBPAS_ACP_TCP_MAIN_H
#define LIBPAS_ACP_TCP_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/poll.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include "../ACP.h"
#include "../../debug.h"

#define ACP_PEER_ID_LENGTH  16
#define ACP_PEER_ADDR_STR_LENGTH  48

#define ACP_DEF_BUFFER(b) char b[ACP_BUFFER_SIZE];memset(b, 0, sizeof b);

//typedef struct {
    //char id[ACP_PEER_ID_LENGTH];
    //char addr_str[ACP_PEER_ADDR_STR_LENGTH];
    //int iid;
    //int port;
    //int fd;
    //int fd_conn;
    //struct sockaddr_in addr;
    //socklen_t addr_size;
//} Peer;
//DEC_LIST(Peer)


extern int acptcp_initServer(int *fd, int port) ;
extern int acptcp_initClient(Peer *peer, __time_t tmo) ;
extern int acptcp_makeClientAddr(struct sockaddr_in *addr, const char *addr_str, int port) ;
extern void acptcp_convertSerialPack(char *pack_str);
extern int acptcp_readPack(int fd, char *buf, size_t length) ;
extern int acptcp_readChannelId ( int fd, int *channel_id);
extern int acptcp_readCmd (int fd, char *buf, size_t length);
extern int acptcp_readData (int fd, char *buf, size_t length) ;
extern int acptcp_send (int fd, const char *pack_str) ;

#endif 

