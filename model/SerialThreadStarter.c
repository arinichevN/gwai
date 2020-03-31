#include "SerialThreadStarter.h"

#define THREAD_STARTER_SLEEP_DELAY (struct timespec) {1,700}

extern SerialThreadLList serial_thread_list;
extern ChannelList channel_list;
extern Mutex serial_thread_list_mutex;

static int hasUnconnectedChannels(ChannelList *list){
	int out = 0;
	FORLi{
		lockMutex(&LIi.mutex);
		if(LIi.thread == NULL){
			out++;
			unlockMutex(&LIi.mutex);
			return out;
		}
		unlockMutex(&LIi.mutex);
	}
	return out;
}

static int fileIsUsed(SerialThreadLList *list, char *path){
	FORLLi(SerialThread){
		if(item->state != TERMINATED && (strcmp(item->serial_path, path) == 0)){
			return 1;
		}
	}
	return 0;
}

static int startSerialThread(int fd, struct timespec cd, char *filename, int rate, int max_retry, ChannelList *rlist, SerialThreadLList *list, Mutex *list_mutex){
	static int id = 0;
	SerialThread *item = stList_getIdleThread(list);
	if(item == NULL){//create new thread
		if(st_addNewToList(list, list_mutex, rlist, id, max_retry, filename, fd, rate, cd)){
			id++;
		}else{
			return 0;
		}
	}else{//use old thread
		st_setParam(item, filename, fd, rate, cd);
	    st_start(item);
	}
	return 1;
    
}

static void prepPort(int fd){
    NANOSLEEP(0,100000000);
}


static void sts_control(SerialThreadStarter *item){
	switch(item->state){
		case SEARCH_NEED://putsdo("TS SEARCH_NEED\n");
			if(hasUnconnectedChannels(&channel_list)){
				item->state = SEARCH_PATH;
			}
			break;
		case SEARCH_PATH:{//putsdo("TS SEARCH_PATH\n");
			item->state = SEARCH_NEED;
			struct dirent *dp;
			while ((dp = readdir(item->dfd)) != NULL)	{
				char *ret = strstr(dp->d_name, item->serial_pattern);	
				if(ret != NULL){		
					char filename[LINE_SIZE*2] ;
					snprintf( filename, sizeof filename, "%s/%s", item->dir, dp->d_name) ;
					//printdo("TS port found: %s\n", filename);
					if(!fileIsUsed(&serial_thread_list, filename)){
						int fd;
						if (!serial_init(&fd, filename, item->serial_rate, item->serial_config)) {
					        printde("failed to initialize serial:%s at rate %d\n", filename, item->serial_rate);
					        continue;
					    }
					    printdo("serial opened: %s %d %s\n", filename, item->serial_rate, item->serial_config);
					    printdo("TS starting thread for port: %s\n", filename);
                        prepPort(fd);
					    startSerialThread(fd, item->thread_cd, filename, item->serial_rate, item->max_retry, &channel_list, &serial_thread_list, &serial_thread_list_mutex);
					}else{
						//putsdo("TS we have already thread for this file\n");
					}
				}
			}
			rewinddir(item->dfd);
			}
			break;
		case INIT:puts("TS INIT");
			item->dir = "/dev";
			if ((item->dfd = opendir(item->dir)) == NULL) {
				printde("can't open %s\n", item->dir);
				return;
			}
			item->state = SEARCH_NEED;
			break;
		default:
			putsde("bad thread starter state\n");
			break;
	}
}

#ifdef MODE_DEBUG
void sts_cleanup_handler ( void *arg ) {
    puts ( "cleaning up thread starter" );
}
#endif

void *sts_main ( void *arg ) {
    SerialThreadStarter *item = arg;
    printdo ( "threadStarter for serial_path: %s has been started\n", item->serial_pattern );
#ifdef MODE_DEBUG
    pthread_cleanup_push ( sts_cleanup_handler, item );
#endif
    while ( 1 ) {
        int old_state;
        if ( threadCancelDisable ( &old_state ) ) {
            sts_control(item);
            threadSetCancelState ( old_state );
        }
        delayTsIdle(THREAD_STARTER_SLEEP_DELAY);
    }
#ifdef MODE_DEBUG
    pthread_cleanup_pop ( 1 );
#endif
}

int sts_init(SerialThreadStarter *item, int max_retry){
	item->dfd = NULL;
	item->dir = "/dev";
	item->max_retry = max_retry;
	item->state = INIT;
	if ( !createMThread ( &item->thread, &sts_main, item ) ) {
		free ( item );
		return 0;
	}
	return 1;
}

void sts_free(SerialThreadStarter *item){
	STOP_THREAD(item->thread);
	closedir(item->dfd);
	item->dfd = NULL;
}
