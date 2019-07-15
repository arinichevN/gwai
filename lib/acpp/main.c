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

#include "../dstructure_auto.h"
#include "../app.h"
#include "../timef.h"

#include "../util.h"

#include "main.h"

int acpp_initServer ( int *fd, int port ) {
    struct sockaddr_in addr;
    if ( ( *fd = socket ( PF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
        perrord ( "socket() failed\n" );
        return 0;
    }
    memset ( ( char * ) &addr, 0, sizeof ( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_port = htons ( port );
    addr.sin_addr.s_addr = htonl ( INADDR_ANY );
    int flag = 1;
    if ( setsockopt ( *fd, IPPROTO_TCP, TCP_NODELAY, ( char * ) &flag, sizeof ( int ) ) < 0 ) {
        perrord ( "setsockopt() failed" );
        close ( *fd );
        return 0;
    }
    if ( bind ( *fd, ( struct sockaddr* ) ( &addr ), sizeof ( addr ) ) == -1 ) {
        perrord ( "bind() failed" );
        close ( *fd );
        return 0;
    }
    if ( listen ( *fd, 7 ) != 0 ) {
        perrord ( "listen() failed" );
        close ( *fd );
        return 0;
    }
    return 1;
}

int acpp_initClient ( Peer *peer, __time_t tmo ) {
    if ( ( peer->fd = socket ( PF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
        perrord ( "socket() failed" );
        return 0;
    }
    struct timeval tv = {tmo, 0};
    if ( setsockopt ( peer->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof ( tv ) ) < 0 ) {
        perrord ( "setsockopt() timeout set failure" );
        close ( peer->fd );
        return 0;
    }
	if ( connect ( peer->fd, ( struct sockaddr* ) &peer->addr, peer->addr_size ) <0 ) {
        perrord ( "connect() failed\n" );
		close(peer->fd);
        return 0;
    }
    return 1;
}

int acpp_makeClientAddr ( struct sockaddr_in *addr, const char *addr_str, int port ) {
    memset ( addr, 0, sizeof ( *addr ) );
    addr->sin_family = AF_INET;
    addr->sin_port = htons ( port );
    if ( inet_aton ( addr_str, & ( addr->sin_addr ) ) == 0 ) {
        perrord ( "inet_aton() failed" );
        return 0;
    }
    return 1;
}


void acpp_dumpBuf ( const char *buf, size_t buf_size ) {
    for ( int i = 0; i < buf_size; i++ ) {
        printf ( "%hhu.", buf[i] );
        if ( buf[i] == '\0' ) {
            putchar ( '\n' );
            return;
        }
    }
    putchar ( '\n' );
}

int acpp_bufCountDataRows ( const char *buf ) {
    int out = 0;
    int len = strlen ( buf );
    for ( int i = 0; i<len && i<ACPP_BUFFER_SIZE; i++ ) {
        if ( buf[i] == ACPP_DELIMITER_ROW ) {
            out++;
        }
    }
    return out;
}

int acpp_getCmdLength ( char *buf ) {
    int out =0;
    for ( int i=0; i<ACPP_BUFFER_SIZE; i++ ) {
        if ( buf[i] == ACPP_DELIMITER_BLOCK ) {
            break;
        }
        out++;
    }
    return out;
}

int acpp_cmdcmp ( char *buf, char * cmd ) {
	int len = acpp_getCmdLength(buf);
    if ( strncmp ( buf, cmd, len ) == 0 ) {
        return 1;
    }
    return 0;
}


int acpp_setCmd ( char * buf, const char *cmd ) {
    int sl = strlen ( cmd );
    if ( sl+1 >= ACPP_BUFFER_SIZE || sl <= 0 ) {
        putsde ( "bad command\n" );
        return 0;
    }
    strcat ( buf, cmd );
    buf[sl] = ACPP_DELIMITER_BLOCK;
    return 1;
}

int acpp_setEnd ( char * buf ) {
    int sl = strlen ( buf );
    if ( sl + 1 < ACPP_BUFFER_SIZE) {
        buf[sl] = ACPP_DELIMITER_END;
        return 1;
    }
    putsde ( "buffer overflow\n" );
    return 0;
}

int acpp_strCat ( char *buf, const char *str ) {
    if ( ( strlen ( buf ) + strlen ( str ) + 1 ) >= ACPP_BUFFER_SIZE ) {
        putsde ( "buffer overflow\n" );
        return 0;
    }
    strcat ( buf, str );
    return 1;
}


int acpp_read ( char *buf, int fd ) {
    ssize_t n = 0;
    do {
        char b[ACPP_PACK_SIZE];
        memset ( b, 0 ,sizeof b );
        n = read ( fd, b, ACPP_PACK_SIZE );
        if ( n == 0 ) {
			putsdo("nothing to read\n");
            break;
        }
        if ( n < 0 ) {
            perrord ( "read()" );
            return 0;
        }
      //  acpp_dumpBuf(b, 16);
        if ( strlen ( buf ) + strlen ( b ) + 1 > ACPP_BUFFER_SIZE ) {
            putsde ( "buffer overflow\n" );
            return 0;
        }
        strcat ( buf, b );
        if( memchr(b, ACPP_DELIMITER_END, ACPP_PACK_SIZE) != NULL){
			putsdo("block delimiter found\n");
			break;
		}
    } while ( 1 );
   // acpp_dumpBuf ( buf, ACPP_BUFFER_SIZE );
    return 1;
}

size_t acpp_readCmd ( char *buf, size_t length, int fd ) {
	memset ( buf, 0 ,sizeof *buf * length);
    char x;
    size_t c = 0;
    while (c < (length-1) && read(fd, &x, 1) == 1) {
        if(x == ACPP_DELIMITER_BLOCK){
			return 1;
		}
		buf[c] = x;
        c++;
    }
    return 0;
}

size_t acpp_readRow ( char *buf, size_t length, int fd ) {
	memset ( buf, 0 ,sizeof *buf * length);
    char x;
    size_t c = 0;
    while (c < (length-1) && read(fd, &x, 1) == 1) {
		if(x == ACPP_DELIMITER_END){
			return 0;
		}
        if(x == ACPP_DELIMITER_ROW){
			//puts("read row");
			//acpp_dumpBuf ( buf, length );
			return 1;
		}
		buf[c] = x;
        c++;
    }
    return 0;
}


int acpp_send ( const char *s, int fd ) {
    int nw = strlen ( s );
    ssize_t nr = write ( fd, s, nw );
    if ( nr < nw ) {
		if(nr < 0){
	        perrord("write() failure");
		}else{
			printde ( "attempt to write %d bytes but %zd written\n", nw, nr );
		}
        return 0;
    }
    //acpp_dumpBuf ( s, nw );
    return 1;
}

#include "data.c"







