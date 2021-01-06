#include "ACPSerial.h"



static void crc_update(uint8_t *crc, uint8_t b) {
    for (int i = 8; i; i--) {
         *crc = ((*crc ^ b) & 1) ? (*crc >> 1) ^ 0x8c : (*crc >> 1);
        b >>= 1;
    }
}

static void crc_updateByStr(uint8_t *crc, const char *str) {
    size_t sz = strlen(str);
    for (size_t i = 0; i < sz; i++) {
        crc_update(crc, (uint8_t) str[i]);
    }
}

void crc_updateByBuf(uint8_t *crc, const char *buf, size_t buf_len) {
    for (size_t i = 0; i < buf_len; i++) {
        crc_update(crc, (uint8_t) buf[i]);
    }
}

int acp_excludeDefinedCRC(uint8_t *v){
	uint8_t out = *v;
	int f = 0;
	while(out == ACP_DELIMITER_START || out == ACP_DELIMITER_COLUMN ||	out == ACP_DELIMITER_END || out == '\0'){
		crc_update(&out, ACP_CRC_EXTRA);
		f = 1;
	}
	*v = out;
	return f;
}

uint8_t acpserial_calcStrCRC(const char *str){
	uint8_t out = ACP_CRC_INI;
	crc_updateByStr(&out, str);
	acp_excludeDefinedCRC(&out);
	return out;
}

uint8_t acp_calcPackCRC(const char *pack_str){
	uint8_t out = ACP_CRC_INI;
	size_t l = strlen(pack_str) - 2;
	crc_updateByBuf(&out, pack_str, l); 
	acp_excludeDefinedCRC(&out);
	return out;
}

int acpserial_addCRC(char *buf, size_t buf_len){
	uint8_t crc = acpserial_calcStrCRC(buf);
	size_t l = strlen(buf);
	if((l + 3) > buf_len) {return 0;}
	buf[l] = crc;
	//printdo("crc: %hhu\n", crc);
	buf[l+1] = ACP_DELIMITER_END;
	buf[l+2] = '\0';
	return 1;
}

int acpserial_tcpToSerial(char *buf, size_t buf_len){
	size_t l1 = strlen(buf);
	if(l1 + 3 > buf_len) {
		printde("short package: buf: %s real: %zd, required: %zd\n", buf, l1 , l1 + 3);
		return 0;
	}
	buf[l1-1] = ACP_DELIMITER_COLUMN;
	return acpserial_addCRC(buf, buf_len);
}

int acpserial_packCheckCRC(const char *str){
	if(ACPSERIAL_CHECK_CRC) {
		size_t sl = strlen(str);
		uint8_t crc1 = (uint8_t) str[sl - 2];
		uint8_t crc2 = acp_calcPackCRC(str);
		if(crc2 != crc1) {
			printde("recieved crc: %hhu(%c), calculated crc: %hhu(%c)\n", crc1,crc1, crc2,crc2);
			return 0;
		}
	}
	return 1;
}

int acpserial_checkChannelIdCRC(const char *str, int channel_id){
	if(!acpserial_packCheckCRC(str)) {
		return ACP_ERROR_CRC;
	}
	int ch_id = channel_id + 1;
	int r = sscanf(str, ACP_DELIMITER_START_STR "%d" ACP_DELIMITER_COLUMN_STR, &ch_id);
	if(r != 1){
		putsde("failed to read channel_id");
		return ACP_ERROR_FORMAT;
	}
	if(ch_id != channel_id){
		putsde("failed to read channel_id");
		return ACP_ERROR_BAD_CHANNEL_ID;
	}
	return ACP_SUCCESS;
}



int acpserial_buildPackSI(char *buf, size_t buf_max_len, char sign, const char *v1, int v2){
	int r = snprintf(buf, buf_max_len, ADSS "%c" ADCS "%s" ADCS "%d" ADCS, sign, v1, v2);
	if(r > 0 && r < (int) buf_max_len){return acpserial_addCRC(buf, buf_max_len);}
	return 0;
}
int acpserial_buildPackSII(char *buf, size_t buf_max_len, char sign, const char *v1, int v2, int v3){
	int r = snprintf(buf, buf_max_len, ADSS "%c" ADCS "%s" ADCS "%d" ADCS "%d" ADCS, sign, v1, v2, v3);
	if(r > 0 && r < (int) buf_max_len){return acpserial_addCRC(buf, buf_max_len);}
	return 0;
}
int acpserial_buildPackSIF(char *buf, size_t buf_max_len, char sign, const char *v1, int v2, double v3){
	int r = snprintf(buf, buf_max_len, ADSS "%c" ADCS "%s" ADCS "%d" ADCS "%f" ADCS, sign, v1, v2, v3);
	if(r > 0 && r < (int) buf_max_len){return acpserial_addCRC(buf, buf_max_len);}
	return 0;
}
int acpserial_buildPackSIS(char *buf, size_t buf_max_len, char sign, const char *v1, int v2, const char *v3){
	int r = snprintf(buf, buf_max_len, ADSS "%c" ADCS "%s" ADCS "%d" ADCS "%s" ADCS, sign, v1, v2, v3);
	if(r > 0 && r < (int) buf_max_len){return acpserial_addCRC(buf, buf_max_len);}
	return 0;
}
int acpserial_buildPackII(char *buf, size_t buf_max_len, char sign, int v1, int v2){
	int r = snprintf(buf, buf_max_len, ADSS "%c" ADCS "%d" ADCS "%d" ADCS, sign, v1, v2);
	if(r > 0 && r < (int) buf_max_len){return acpserial_addCRC(buf, buf_max_len);}
	return 0;
}
int acpserial_buildPackIII(char *buf, size_t buf_max_len, char sign, int v1, int v2, int v3){
	int r = snprintf(buf, buf_max_len, ADSS "%c" ADCS "%d" ADCS "%d" ADCS "%d" ADCS, sign, v1, v2, v3);
	if(r > 0 && r < (int) buf_max_len){return acpserial_addCRC(buf, buf_max_len);}
	return 0;
}
int acpserial_buildPackIUl(char *buf, size_t buf_max_len, char sign, int v1, unsigned long v2){
	int r = snprintf(buf, buf_max_len, ADSS "%c" ADCS "%d" ADCS "%lu" ADCS, sign, v1, v2);
	if(r > 0 && r < (int) buf_max_len){return acpserial_addCRC(buf, buf_max_len);}
	return 0;
}
int acpserial_buildPackIF(char *buf, size_t buf_max_len, char sign, int v1, double v2){
	int r = snprintf(buf, buf_max_len, ADSS "%c" ADCS "%d" ADCS "%f" ADCS, sign, v1, v2);
	if(r > 0 && r < (int) buf_max_len){return acpserial_addCRC(buf, buf_max_len);}
	return 0;
}
int acpserial_buildPackIIF(char *buf, size_t buf_max_len, char sign, int v1, int v2, double v3){
	int r = snprintf(buf, buf_max_len, ADSS "%c" ADCS "%d" ADCS "%d" ADCS "%f" ADCS, sign, v1, v2, v3);
	if(r > 0 && r < (int) buf_max_len){return acpserial_addCRC(buf, buf_max_len);}
	return 0;
}
int acpserial_buildPackIS(char *buf, size_t buf_max_len, char sign, int v1, const char *v2){
	int r = snprintf(buf, buf_max_len, ADSS "%c" ADCS "%d" ADCS "%s" ADCS, sign, v1, v2);
	if(r > 0 && r < (int) buf_max_len){return acpserial_addCRC(buf, buf_max_len);}
	return 0;
}
int acpserial_buildPackIIS(char *buf, size_t buf_max_len, char sign, int v1, int v2, const char *v3){
	int r = snprintf(buf, buf_max_len, ADSS "%c" ADCS "%d" ADCS "%d" ADCS "%s" ADCS, sign, v1, v2, v3);
	if(r > 0 && r < (int) buf_max_len){return acpserial_addCRC(buf, buf_max_len);}
	return 0;
}
int acpserial_buildPackIFTS(char *buf, size_t buf_max_len, char sign, int v1, FTS *v2){
	int r = snprintf(buf, buf_max_len, ADSS "%c" ADCS "%d" ADCS "%f" ADCS "%ld" ADCS "%ld" ADCS "%d" ADCS, sign, v1, v2->value, v2->tm.tv_sec, v2->tm.tv_nsec, v2->state);
	if(r > 0 && r < (int) buf_max_len){return acpserial_addCRC(buf, buf_max_len);}
	return 0;
}

int acpserial_send(int fd, const char *buf){
	printdo("%s\n\n", buf);
	ssize_t n = write(fd, buf, strlen(buf));
    if (n < strlen(buf)) {
		if(n == -1){
			if(!serial_alive(fd)){
				putsde("file not found\n");
				return ACP_ERROR_DEVICE_DISCONNECTED;
			}
			perror("write()");
			return ACP_ERROR_CONNECTION;
		}
		printde("expected to write %zu but %zd done\n", strlen(buf), n);
        return ACP_ERROR_CONNECTION;
    }
    return ACP_SUCCESS;
}

int acpserial_readResponse(int fd, char *buf, size_t buf_len){
	if(!serial_canRead(fd, ACPSERIAL_TIMEOUT_MS)){
		putsde("failed to read slave response\n");
		return ACP_ERROR_NO_RESPONSE;
		//return ACP_ERROR_CONNECTION;
	}
	size_t n = serial_readUntil(fd, buf, buf_len - 1, ACP_DELIMITER_END);
    tcflush(fd, TCIOFLUSH);
    printdo("slave response: %s \tsize = %zu\n", buf, n);
	if(n == 0){
		if(!serial_alive(fd)){
			putsde("file not found\n");
			return ACP_ERROR_DEVICE_DISCONNECTED;
		}
		putsde("no response from slave\n");
		return ACP_ERROR_NO_RESPONSE;
	}
	if(strlen(buf) < ACP_PACK_MIN_LENGTH) return ACP_ERROR_SHORT_RESPONSE;
	if(!acpserial_packCheckCRC(buf)){
		putsde("bad CRC\n");
		return ACP_ERROR_CRC;
	}
	return ACP_SUCCESS;
}

int acpserial_readRequest(int fd, char *buf, size_t buf_len){
	size_t c = 0;
	int pack_detected = 0;
	Ton pack_tmr_n; Ton *pack_tmr = &pack_tmr_n;
	ton_setInterval((struct timespec){0, ACP_PACK_TIMEOUT_MS * 1000000}, pack_tmr);
	tcflush(fd, TCIFLUSH);
	while(1){
		if (c >= (buf_len - 2)) return ACP_ERROR_BUFFER_OVERFLOW;
		char x;
		//putsdo("reading...\n");
		ssize_t r = read(fd, &x, 1);
		//printdo("\treturn: %zd\n", r);
		if(r == 0){
			if(!serial_alive(fd)){
				putsde("file not found\n");
				return ACP_ERROR_DEVICE_DISCONNECTED;
			}
			//putsdo("no request\n");
			return ACP_NO_DATA;
		}
		if(r != 1){
			putsde("read failed: ");
			perror("read()");
			return ACP_ERROR_READ;
		}
		if(!pack_detected){if(x == ACP_DELIMITER_START){pack_detected = 1;ton_reset(pack_tmr);}else{putsde("noise\n"); return ACP_NOISE;}}
		if(ton(pack_tmr)) {putsde("pack timeout\n"); return ACP_ERROR_PACK_TIMEOUT;}
		buf[c] = x; c++;
		if (x == ACP_DELIMITER_END) {
			if (c < ACP_PACK_MIN_LENGTH) {
				putsde(" short pack\n"); 
				return ACP_ERROR_SHORT_PACKAGE;
			}
			if(!acpserial_packCheckCRC(buf)){
				putsde("bad CRC\n");
				return ACP_ERROR_CRC;
			}
			printdo("request: %s\n", buf);
			return ACP_SUCCESS;
	    }
	}
	putsde("unexpected while break\n");
	return ACP_ERROR;
}

int acpserial_sendTcpPack(int fd, char *pack_str){printdo("tcp pack: %s\n", pack_str);
	if(!acpserial_tcpToSerial(pack_str, ACP_BUF_MAX_LENGTH)){
		return ACP_ERROR_FORMAT;
	}
	return acpserial_send(fd, pack_str);
}

int acpserial_sendChCmd(int fd, char sign, int cmd, int channel_id){
	size_t blen=40;
	char buf[blen];
	acpserial_buildPackII(buf, blen, sign, cmd, channel_id);
	return acpserial_send(fd, buf);
}

int acpserial_sendChCmdF1(int fd, char sign, int cmd, int channel_id, double v){
	size_t blen=64;
	char buf[blen];
	acpserial_buildPackIIF(buf, blen, sign, cmd, channel_id, v);
	return acpserial_send(fd, buf);
}

int acpserial_sendChCmdI1(int fd, char sign, int cmd, int channel_id, int v){
	size_t blen=64;
	char buf[blen];
	acpserial_buildPackIII(buf, blen, sign, cmd, channel_id, v);
	return acpserial_send(fd, buf);
}

int acpserial_sendChCmdStr(int fd, char sign, int cmd, int channel_id, const char *v){
	size_t blen=64;
	char buf[blen];
	acpserial_buildPackIIS(buf, blen, sign, cmd, channel_id, v);
	return acpserial_send(fd, buf);
}

int acpserial_extractFTS(const char *buf, size_t len, FTS *v){
	if(!acpserial_packCheckCRC(buf)) {
		return ACP_ERROR_CRC;
	}
	FTS sfts;
	int r = sscanf(buf, ADSS ASRES ADCS "%d" ADCS "%lf" ADCS "%ld" ADCS "%ld" ADCS "%d" ADCS, &sfts.id, &sfts.value, &sfts.tm.tv_sec, &sfts.tm.tv_nsec, &sfts.state);
	int nr = 4;
	if(r != nr){
		printde("failed to parse response (found:%d, need:%d)\n", r, nr);
		return 0;
	}
	*v = sfts;
	return ACP_SUCCESS;
}

int acpserial_extractI2(const char *buf, size_t len, int *v1, int *v2){
	if(!acpserial_packCheckCRC(buf)) {
		putsde("failed to check CRC\n");
		return ACP_ERROR_CRC;
	}
	int vv1, vv2;
	int r = sscanf(buf, ADSS ASRES ADCS "%d" ADCS "%d" ADCS, &vv1, &vv2);
	int nr = 2;
	if(r != nr){
		printde("failed to parse response (found:%d, need:%d)\n", r, nr);
		return ACP_ERROR_FORMAT;
	}
	*v1 = vv1;
	*v2 = vv2;
	return ACP_SUCCESS;
}

int acpserial_extractSI(const char *buf, size_t len, char *v1, size_t v1_len, int *v2){
	if(!acpserial_packCheckCRC(buf)) {
		putsde("failed to check CRC\n");
		return ACP_ERROR_CRC;
	}
	if(buf[0] != ACP_DELIMITER_START) return ACP_ERROR_FORMAT;
	if(buf[1] != ACP_SIGN_RESPONSE) return ACP_ERROR_FORMAT;
	char vv1[v1_len];
	memset(vv1, 0, v1_len);
	size_t c = 0;
	for(size_t i=3;i<len;i++){
		if(buf[i] == ACP_DELIMITER_COLUMN) {c = i; break;}
		if(!acp_checkStrColumnChar(buf[i])){
			return ACP_ERROR_FORMAT;
		}
		if(c >= v1_len) return ACP_ERROR_FORMAT;
		vv1[c] = buf[i]; c++;
	}
	if(c < 1) return ACP_ERROR_FORMAT;
	int vv2;
	int r = sscanf(&buf[c], ADCS "%d" ADCS, &vv2);
	int nr = 1;
	if(r != nr){
		printde("failed to parse response (found:%d, need:%d)\n", r, nr);
		return ACP_ERROR_FORMAT;
	}
	strncpy(v1, vv1, v1_len);
	*v2 = vv2;
	return ACP_SUCCESS;
}







