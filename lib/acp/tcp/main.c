#include "main.h"

int acptcp_initServer ( int *fd, int port ) {
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

int acptcp_initClient ( Peer *peer, __time_t tmo ) {
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

int acptcp_makeClientAddr ( struct sockaddr_in *addr, const char *addr_str, int port ) {
    memset ( addr, 0, sizeof ( *addr ) );
    addr->sin_family = AF_INET;
    addr->sin_port = htons ( port );
    if ( inet_aton ( addr_str, & ( addr->sin_addr ) ) == 0 ) {
        perrord ( "inet_aton() failed" );
        return 0;
    }
    return 1;
}

void acptcp_convertSerialPack(char *pack_str){
	size_t len = strlen(pack_str);
	for(size_t i = 0;i < len;i++){
		if(i > 1 && pack_str[i] == ACP_DELIMITER_END){
			pack_str[i] = '\0';
			pack_str[i - 1] = '\0';
			pack_str[i - 2] = ACP_DELIMITER_END;
			break;
		}
	}
}

int acptcp_readPack(int fd, char *buf, size_t length) {
	memset ( buf, 0 ,(sizeof *buf) * length);
    size_t c = 0;
    ssize_t lim = length-1;
    int start_detected = 0;
	while(1){
		if(c >= lim){
			printde("%zu < %zd\n", c, lim);
		   break;
		}
		char x;
		ssize_t n = read(fd, &x, 1);
		if(n != 1){
			printde("reading error: read() returned %zd\n", n);
			break;
		}
		if(c == 0 && x == ACP_DELIMITER_START){
			start_detected = 1;
			buf[c] = x;c++;
			continue;
		}
		if(start_detected){
			buf[c] = x;c++;
	        if(x == ACP_DELIMITER_END){
				printdo("tcp pack: %s\n", buf);
				return 1;
			}
		}else{
			putsde("ACP_DELIMITER_START not detected\n");
			return 0;
		}
    }
    return 0;
}

int acptcp_readCmd (int fd, char *buf, size_t length) {
	memset ( buf, 0 ,(sizeof *buf) * length);
    size_t c = 0;
    ssize_t lim = length-1;
    int start_detected = 0;
	while(1){
		if(c >= lim){
			printde("%zu < %zd\n", c, lim);
		   break;
		}
		char x;
		ssize_t n = read(fd, &x, 1);
		if(n != 1){
			printde("reading error: read() returned %zd\n", n);
			break;
		}
		if(c == 0 && x == ACP_DELIMITER_START){
			start_detected = 1;
			continue;
		}
		if(start_detected){
	        if(x == ACP_DELIMITER_COLUMN || x == ACP_DELIMITER_END){
				printdo("tcp cmd = %s\n", buf);
				return 1;
			}
			buf[c] = x;
	        c++;
		}else{
			putsde("ACP_DELIMITER_START not detected\n");
			return 0;
		}
    }
    return 0;
}


int acptcp_readChannelId (int fd, int *channel_id  ) {
	char buf[ACP_CHANNEL_ID_STRLEN];
	memset ( buf, 0 ,(sizeof *buf) * ACP_CHANNEL_ID_STRLEN);
    ssize_t c = 0;
   // while (c < (ACP_CHANNEL_ID_STRLEN-1) && (read(fd, &x, 1) == 1)) {
   ssize_t lim = ACP_CHANNEL_ID_STRLEN-1;
   while(1){
		if(c >= lim){
			printde("%zd < %zd\n", c, lim);
			break;
		}
		char x;
		ssize_t n = read(fd, &x, 1);
		if(n != 1){
			printde("reading error: read() returned %zd\n", n);
			break;
		}
        if(x == ACP_DELIMITER_COLUMN){
			*channel_id = atoi(buf);
			printdo("tcp channel_id = %d\n", *channel_id);
			return 1;
		}
		buf[c] = x;
        c++;
    }
    return 0;
}

//int acptcp_readData (int fd, char *buf, size_t length) {
	//memset ( buf, 0 ,(sizeof *buf) * length);
    //char x;
    //size_t c = 0;
    //while (c < (length-1) && (read(fd, &x, 1) == 1)) {
        //if(x == ACP_DELIMITER_END){
			//return 1;
		//}
		//buf[c] = x;
        //c++;
    //}
    //return 0;
//}


int acptcp_send ( const char *s, int fd ) {
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
    //acp_dumpBuf ( s, nw );
    return 1;
}
