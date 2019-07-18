#include "puart.h"


#define PUART_TIMEOUT_MS 500

#define PUART_CMD_SELECTED "selected"



int puart_readResponse(int fd, char *buf, int len){
	if(!serial_canRead(fd, PUART_TIMEOUT_MS)){
		putsde("failed to read slave response\n");
		return PUART_NO_RESPONSE;
	}
	size_t n = serial_readUntil(fd, buf, len, PUART_DELIMITER_END);
    tcflush(fd,TCIOFLUSH);
	if(n == 0){
		struct stat stb;
		fstat(fd, &stb);
		if(stb.st_nlink <= 0){
			putsde("file not found\n");
			return PUART_CONNECTION_FAILED;
		}
		putsde("no response from slave\n");
		return PUART_NO_RESPONSE;
	}
    printdo("slave response: %s\n", buf);
	return PUART_SUCCESS;
}


int puart_sendDouble(int fd, int channel_id, char *cmd, double v){
	tcflush(fd,TCIOFLUSH);
	size_t blen=64;
	char buf[blen];
	snprintf ( buf, sizeof buf, "select" PUART_DELIMITER_END_STR "%d" PUART_DELIMITER_END_STR "%s" PUART_DELIMITER_END_STR "%f" PUART_DELIMITER_END_STR,channel_id, cmd, v );
	printdo("%s\n", buf);
	ssize_t n = write(fd, buf, strlen(buf));
    if (n < strlen(buf)) {
		if(n==PUART_CONNECTION_FAILED){
			perror("write()");
			return PUART_CONNECTION_FAILED;
		}
		printde("expected to write %d but %d done\n", strlen(buf), n);
        return 0;
    }
    return PUART_SUCCESS;
}

int puart_sendInt(int fd, int channel_id, char *cmd, int v){
	tcflush(fd,TCIOFLUSH);
	size_t blen=64;
	char buf[blen];
	snprintf ( buf, sizeof buf, "select" PUART_DELIMITER_END_STR "%d" PUART_DELIMITER_END_STR "%s" PUART_DELIMITER_END_STR "%d" PUART_DELIMITER_END_STR,channel_id, cmd, v );
	printdo("%s\n", buf);
	ssize_t n = write(fd, buf, strlen(buf));
    if (n < strlen(buf)) {
		if(n==PUART_CONNECTION_FAILED){
			perror("write()");
			return PUART_CONNECTION_FAILED;
		}
		printde("expected to write %d but %d done\n", strlen(buf), n);
        return 0;
    }
    return PUART_SUCCESS;
}

int puart_sendStr(int fd, int channel_id, char *cmd, char *v){
	//tcflush(fd,TCIOFLUSH);
	size_t blen=64;
	char buf[blen];
	snprintf ( buf, sizeof buf, "select" PUART_DELIMITER_END_STR "%d" PUART_DELIMITER_END_STR "%s" PUART_DELIMITER_END_STR "%s" PUART_DELIMITER_END_STR,channel_id, cmd, v );
	printdo("%s\n", buf);
	ssize_t n = write(fd, buf, strlen(buf));
    if (n < strlen(buf)) {
		if(n==PUART_CONNECTION_FAILED){
			perror("write()");
			return PUART_CONNECTION_FAILED;
		}
		printde("expected to write %d but %d done\n", strlen(buf), n);
        return 0;
    }
    return PUART_SUCCESS;
}

int puart_sendCmd (int fd, int channel_id, char *cmd){
	if(!serial_canWrite(fd, PUART_TIMEOUT_MS)){
		putsde("failed to read slave response\n");
		return PUART_CONNECTION_FAILED;
	}
	tcflush(fd,TCIOFLUSH);
	size_t blen=64;
	char buf[blen];memset(buf, 0, sizeof buf);
	snprintf ( buf, sizeof buf, "select" PUART_DELIMITER_END_STR "%d" PUART_DELIMITER_END_STR "%s" PUART_DELIMITER_END_STR,channel_id, cmd );
	printdo("%s\n", buf);
	ssize_t n = write(fd, buf, strlen(buf));
    if (n < strlen(buf)) {
		if(n==PUART_CONNECTION_FAILED){
			perror("write()");
			return PUART_CONNECTION_FAILED;
		}
		printde("expected to write %d but %d done\n", strlen(buf), n);
        return 0;
    }
    return PUART_SUCCESS;
}

int puart_channelExist (int fd, int channel_id){
	int r = puart_sendCmd (fd, channel_id, PUART_CMD_SELECTED);
	if(r != PUART_SUCCESS){
		printde("communication error where channel_id=%d\n", channel_id);
		puart_sendEnd(fd);
		return r;
	}
	size_t resp_len = 5;
	char response[resp_len];
    memset(response, 0, resp_len);
	r = puart_readResponse(fd, response, resp_len);
	puart_sendEnd(fd);
    if(r != PUART_SUCCESS){
		printde("communication error where channel_id=%d\n", channel_id);
		return r;
	}
	int id=-1;
	r = sscanf(response, "%d" PUART_DELIMITER_END_STR, &id );
	if(r==1){
		if(channel_id == id){
			return 1;
		}else{
			printde("bad channel_id=%d, but %d expected\n", id, channel_id);
		}
	}else{
		putsde("failed to parse response\n");
	}
	return 0;
}






