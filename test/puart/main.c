#include "main.h"

static int app_state = APP_INIT;
static int fd=-1;

#define REQUEST_IS(v) (strcmp(cmd_str, v)==0)
#define CMD_GET_FTS "cgfts"

#define FV "%lf"
#define PDB PUART_DELIMITER_BLOCK_STR
#define PDE PUART_DELIMITER_END_STR

int puar_sendCmd (int fd, int channel_id, char *cmd){
	tcflush(fd,TCIOFLUSH);
	size_t blen=64;
	char buf[blen];
	snprintf ( buf, sizeof buf, "select" PUART_DELIMITER_END_STR "%d" PUART_DELIMITER_END_STR "%s" PUART_DELIMITER_END_STR "end" PUART_DELIMITER_END_STR,channel_id, cmd );
	printdo("%s\n", buf);
	ssize_t n = write(fd, buf, strlen(buf));
	//char *q="select;1;cgfts;end;";
	//ssize_t n = write(fd, q, strlen(q));
    if (n < strlen(buf)) {
		if(n==PUART_CONNECTION_FAILED){
			perror("write()");
			return PUART_CONNECTION_FAILED;
		}
		printde("expected to write %d but %d done\n", strlen(buf), n);
        return 0;
    }
    return 1;
}

int channelGetFTSData ( int channel_id, int fd ) {
	int r = puar_sendCmd (fd, channel_id, CMD_GET_FTS);
	if(r == PUART_CONNECTION_FAILED){
		return PUART_CONNECTION_FAILED;
	}
	size_t resp_len = 128;
	char response[resp_len];
    memset(response, 0, resp_len);
	r = puart_readResponse(fd, response, resp_len);
    if(r == PUART_CONNECTION_FAILED){
		return PUART_CONNECTION_FAILED;
	}
	if(!r){
		putsde("read fts: failed to read response\n");
		return 0;
	}
	int id=-1;
	int tm, state;
	double v;
	r = sscanf(response, "%d" PDB FV PDB "%d" PDB "%d" PDE, &id, &v, &state, &tm);
	int nr = 4;
	if(r != nr){
		printde("read fts: failed to parse response (found:%d, need:%d)\n", r, nr);
		return 0;
	}
	
	int exp = 4;
	if(r!=exp){
		printde("read fts: expected %d parameters but found %d\n", exp, r);
		return 0;
	}
	printf("id:%d v:%f state:%d tm:%d\n", id, v, state, tm);
	if(id != channel_id){
		printde("read fts: expected %d id but returned %d\n", channel_id, id);
		return 0;
	}
	return 1;
}
#define GPNB int bytes;ioctl(fd, FIONREAD, &bytes);printf("bytes available: %d\n", bytes);
void readAB(int fd){
	int bytes;ioctl(fd, FIONREAD, &bytes);printf("bytes available: %d\n", bytes);
	if(bytes <=0 ) return;
	char buf[bytes+1];
	int n = read(fd, buf, bytes);
	buf[n]='\0';
	puts(buf);
}

void readT(int fd){
	char buf[255];
	int n = read(fd, buf, 255);
	buf[n]='\0';
	puts(buf);
}

void readD(int fd){
	char x;
    while (read(fd, &x, 1) == 1) {
        putchar(x);
        if(x == ';') break;
    }
    putchar('\n');
    tcflush(fd, TCIFLUSH);//flushes data received but not read
}

void appRun (int *state, int init_state) {
	puts("\n-----------------------------");
	puts("your command:");
	char cmd_str[31];
	memset(cmd_str, 0, sizeof cmd_str);
	int channel_id = -1;
	int n = fscanf(stdin, "%16s", cmd_str);
	if(n!=1){
		puts("failed to read your command");
		return ;
	}
	if(REQUEST_IS("gfts")){
		channelGetFTSData ( channel_id, fd );
	}else if(REQUEST_IS("selected")){
		puart_channelExist(fd, channel_id);
	}else if(REQUEST_IS("z")){
		puart_channelExist (fd, 3);
		return;
	}else if(REQUEST_IS("x")){// no soft no hard flow cntrl, 8N1 
		serialPuts(fd, "select;3;cgfts;end;");
		char buf[99];memset(buf, 0, 99);
		sleep(1);
		GPNB
		serialReadUntil(fd, buf, 99, ';');
		puts(buf);
		return;
	}else if(REQUEST_IS("c")){
		for(int i=0;i<8;i++){
			char req[64];
			snprintf(req, 64, "select;%d;cgfts;", i+1);
			serialPuts(fd, req);
			delayUsIdle(300000);
			readAB(fd);
			serialPuts(fd, "end;");
			delayUsIdle(300000);
		}
		return;
	}else if(REQUEST_IS("b")){
		for(int i=0;i<8;i++){
			char req[64];
			snprintf(req, 64, "select;%d;cgfts;", i+1);
			serialPuts(fd, req);
			//delayUsIdle(300000);
			readD(fd);
			serialPuts(fd, "end;");
			//delayUsIdle(300000);
		}
		return;
	}else if(REQUEST_IS("v")){
		for(int i=0;i<1;i++){
			char req[64];
			snprintf(req, 64, "select;%d;cgfts;", i+1);
			serialPuts(fd, req);
			//delayUsIdle(300000);
			readD(fd);
			serialPuts(fd, "end;");
			//delayUsIdle(300000);
		}
		return;
	}else{
		puts("unknown command");
	}
	puts("");
	
}

int startSerial(int *fdp){
	char cmd_str[31];
	memset(cmd_str, 0, sizeof cmd_str);
	FILE *fl = fopen("./test/puart/config.tsv","r");
	if(fl==NULL){
		perror("fopen()");
		return 0;
	}
	int id = 0;
	int rate = 0;
	int n = fscanf(fl, "%d\t%d", &id, &rate);
	if(n!=2){
		putsde("failed to read config file");
		return 0;
	}
	if(id < 0){
	   putsde("bad serial path id");
	   return 0;
	}
	char sp[LINE_SIZE];
	memset(sp, 0, sizeof sp);
	snprintf(sp, LINE_SIZE, "/dev/ttyUSB%d", id);
	if (!initSerial(fdp, sp, rate)) {
        putsde("failed to initialize serial\n");
        return 0;
    }
    printf("connected to %s at %d rate", sp, rate);
    return 1;
}

int initApp() {
	if (!startSerial(&fd)) {
        putsde("failed to initialize serial\n");
        return 0;
    }
   return 1;
}

int initData() {

   return 1;
}

void freeData() {

}

void freeApp() {
	close(fd);
   freeData();
  
}

void exit_nicely () {
   freeApp();
   putsdo ("\nexiting now...\n");
   exit (EXIT_SUCCESS);
}

int main (int argc, char** argv) {
	//THREAD_SET_PRIORITY_MAX(SCHED_FIFO)

#ifndef MODE_DEBUG
   daemon (0, 0);
#endif
   conSig (&exit_nicely);
   int data_initialized = 0;
   while (1) {
      switch (app_state) {
         case APP_INIT:
            if (!initApp()) {
               return (EXIT_FAILURE);
            }
            app_state = APP_INIT_DATA;
            break;
         case APP_INIT_DATA:
            data_initialized = initData();
            app_state = APP_RUN;
            break;
         case APP_RUN:
            appRun (&app_state, data_initialized);
            break;
         case APP_STOP:
            freeData();
            data_initialized = 0;
            app_state = APP_RUN;
            break;
         case APP_RESET:
            freeApp();
            data_initialized = 0;
            app_state = APP_INIT;
            break;
         case APP_EXIT:
            exit_nicely();
            break;
         default:
            freeApp();
            putsde ("unknown application state\n");
            return (EXIT_FAILURE);
      }
   }
   freeApp();
   return (EXIT_SUCCESS);
}

