#include "main.h"

static int app_state = APP_INIT;
static int fd=-1;

#define REQUEST_IS(v) (strcmp(cmd_str, v)==0)
#define CMD_GET_FTS "cgfts"

#define FV "%lf"



int channelExists ( int channel_id, int fd ) {
	printdo("Exists? id=%d\n", channel_id);
	int r = acpserial_sendChCmd (fd, channel_id, ACP_CMD_CHANNEL_EXISTS);
	if(r == ACP_ERROR_CONNECTION){
		return r;
	}
	size_t resp_len = 128;
	char response[resp_len];
    memset(response, 0, resp_len);
    
    struct timespec tm = {0, 10000000};
    nanosleep(&tm, NULL);
    
    r = acpserial_readResponse(fd, response, resp_len);
    if(r != ACP_SUCCESS){
		printde("\tcommunication error where channel_id=%d\n", channel_id);
		return r;
	}
	int id, exs = 0;
    r = acpserial_extractI2(response, resp_len, &id, &exs);
    if(r != ACP_SUCCESS){
		printde("\tparsing error where channel_id=%d\n", channel_id);
		return r;
	}
	if(exs != ACP_CHANNEL_EXISTS){
		printde("channel %d returned not exists\n", channel_id);
		return ACP_ERROR;
	}
    if(id != channel_id){
		printde("expected %d id but returned %d\n", channel_id, id);
		return ACP_ERROR_BAD_CHANNEL_ID;
	}
    return ACP_SUCCESS;
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
	puts("your id:command:");
	char cmd_str[31];
	memset(cmd_str, 0, sizeof cmd_str);
	int channel_id = -1;
	int n = fscanf(stdin, "%d:%16s", &channel_id, cmd_str);
	if(n!=2){
		puts("failed to read your command");
		return ;
	}
	if(REQUEST_IS("gfts")){
		channelExists ( channel_id, fd );
	}else if(REQUEST_IS("selected")){
		
	}else if(REQUEST_IS("z")){
		
		return;
	}else if(REQUEST_IS("x")){// no soft no hard flow cntrl, 8N1 
		
		return;
	}else if(REQUEST_IS("c")){
		for(int i=0;i<8;i++){
			char req[64];
			snprintf(req, 64, "select;%d;cgfts;", i+1);
			serial_puts(fd, req);
			delayUsIdle(300000);
			readAB(fd);
			serial_puts(fd, "end;");
			delayUsIdle(300000);
		}
		return;
	}else if(REQUEST_IS("b")){
		for(int i=0;i<8;i++){
			char req[64];
			snprintf(req, 64, "select;%d;cgfts;", i+1);
			serial_puts(fd, req);
			//delayUsIdle(300000);
			readD(fd);
			serial_puts(fd, "end;");
			//delayUsIdle(300000);
		}
		return;
	}else if(REQUEST_IS("v")){
		for(int i=0;i<1;i++){
			char req[64];
			snprintf(req, 64, "select;%d;cgfts;", i+1);
			serial_puts(fd, req);
			//delayUsIdle(300000);
			readD(fd);
			serial_puts(fd, "end;");
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
    char config[4];config[3]='\0';
	int n = fscanf(fl, "%d\t%d\t%3s", &id, &rate, config);
	if(n!=3){
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
	if (!serial_init(fdp, sp, rate, config)) {
        putsde("failed to initialize serial\n");
        return 0;
    }
    printf("connected to %s at %d rate and config %s\n", sp, rate, config);
    serial_printOptions(*fdp);
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

