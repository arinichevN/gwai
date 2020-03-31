
#ifndef LIBPAS_APP_H
#define LIBPAS_APP_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <sched.h>

#include <signal.h>

#include <pthread.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>

#include "common.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define GOOD_FLOAT 1.0
#define BAD_FLOAT 0.0
#define GOOD_INT 1
#define BAD_INT 0

#define POSITIVE_FLOAT GOOD_FLOAT
#define NEGATIVE_FLOAT BAD_FLOAT
#define POSITIVE_INT GOOD_INT
#define NEGATIVE_INT BAD_INT

#define CDS ACP_DELIMITER_COLUMN_STR
#define RDS ACP_DELIMITER_ROW_STR

#define DEF_THREAD pthread_t thread;char thread_cmd=0;void *threadFunction(void *arg);
#define THREAD_CREATE createThread(&thread,&threadFunction,&thread_cmd)
#define THREAD_STOP thread_cmd = 1;pthread_join(thread, NULL);
#define THREAD_EXIT_ON_CMD if (*cmd) {*cmd = 0;return (EXIT_SUCCESS); }
#define THREAD_DEF_CMD char *cmd = (char *) arg;
#define THREAD_SET_PRIORITY_MAX(p) struct sched_param param;param.sched_priority = sched_get_priority_max (p);pthread_setschedparam (pthread_self (), p, &param);
#define THREAD_SET_PRIORITY_MAXM(p,m) struct sched_param param;param.sched_priority = sched_get_priority_max (p) - m;pthread_setschedparam (pthread_self (), p, &param);

#define PROG_ERROR_NO_SIGNAL_FROM_CLIENT 0x1
#define PROG_ERROR_NO_RESPONSE_FROM_SENSOR 0x2
#define PROG_ERROR_HARDWARE 0x4

#define BIT_ENABLE(buf,v) (buf)|=(v)
#define BIT_DISABLE(buf,v) (buf)&=~(v)
#define BIT_IS_ENABLED(buf,v) (buf)&(v)

#define FORLi for (size_t i = 0; i < list->length; i++) 
#define FORL FORLi
#define FORMLi for (size_t i = 0; i < list->max_length; i++) 
#define FORLISTP(L, I) for (size_t I = 0; I < (L)->length; I++) 
#define FORLISTN(L, I) for (size_t I = 0; I < (L).length; I++) 
#define FORLISTMP(L, I) for (size_t I = 0; I < (L)->max_length; I++) 
#define FORLISTMN(L, I) for (size_t I = 0; I < (L).max_length; I++) 
#define FORLIST(I) for (size_t I = 0; I < list->length; I++) 
#define FORLLj  for (size_t j = i + 1; j < list->length; j++) 
#define FORLISTPL(L, I, J)  for (size_t J = I + 1; J < (L)->length; J++) 
#define FORLISTNL(L, I, J)  for (size_t J = I + 1; J < (L).length; J++) 
#define FFORLISTPL(L, I, J)  for (size_t I = 0; I < (L)->length; I++){for (size_t J = I + 1; J < (L)->length; J++)
#define FFORLISTNL(L, I, J)  for (size_t I = 0; I < (L).length; I++){for (size_t J = I + 1; J < (L).length; J++)

#define FOREACH_CHANNEL FOREACH_LLIST(item,&channel_list,Channel)

#define LI(L,I) (L)->item[I]
#define LIi LI(list,i)
#define LIj LI(list,j)
#define LIll list->item[list->length]
#define Lil list->length-1
#define LL list->length
#define LML list->max_length
#define LIiei(v) LIi.v=atoi(v)
#define LIief(v) LIi.v=atof(v)


#ifdef MODE_DEBUG
#define STOP_CHANNEL_THREAD(channel) {printf("signaling thread %d to cancel...\n", (channel)->id);if (pthread_cancel((channel)->thread) != 0)perror("pthread_cancel()");void * App_result;printf("joining thread %d...\n", (channel)->id);if (pthread_join((channel)->thread, &App_result) != 0) perror("pthread_join()");if (App_result != PTHREAD_CANCELED) printf("thread %d not canceled\n", (channel)->id);}
#else
#define STOP_CHANNEL_THREAD(channel) {pthread_cancel((channel)->thread);void * App_result;pthread_join((channel)->thread, &App_result);}
#endif

#ifdef MODE_DEBUG
#define STOP_ALL_LLIST_THREADS(list, T) {FOREACH_LLIST(item,(list),T){printf("signaling thread %d to cancel...\n", item->id);while (pthread_cancel(item->thread) != 0){;}}FOREACH_LLIST(item,list,T){void * App_result;printf("joining thread %d...\n", item->id);if (pthread_join(item->thread, &App_result) != 0) perror("pthread_join()");if (App_result != PTHREAD_CANCELED) printf("thread %d not canceled\n", item->id);}}
#else
#define STOP_ALL_LLIST_THREADS(list, T) {FOREACH_LLIST(item,(list),T){while(pthread_cancel(item->thread)!=0){;}}FOREACH_LLIST(item,list,T){void * App_result;pthread_join(item->thread, &App_result);}}
#endif
#define STOP_ALL_CHANNEL_THREADS(channel_list) STOP_ALL_LLIST_THREADS(channel_list, Channel)

#ifdef MODE_DEBUG
#define STOP_ALL_LIST_THREADS(list) FORLISTP(list, i){printf("signaling thread %d to cancel...\n", (list)->item[i].id);if (pthread_cancel((list)->item[i].thread) != 0) perror("pthread_cancel()");}FORLISTP(list, i){void * App_result;printf("joining thread %d...\n", (list)->item[i].id);if (pthread_join((list)->item[i].thread, &App_result) != 0) perror("pthread_join()");if (App_result != PTHREAD_CANCELED) printf("thread %d not canceled\n", (list)->item[i].id);}
#else
#define STOP_ALL_LIST_THREADS(list) FORLISTP(list, i){pthread_cancel((list)->item[i].thread);} FORLISTP(list, i){void * result;pthread_join((list)->item[i].thread, &result);}
#endif

//#define STOP_THREAD(item) pthread_cancel((item));pthread_join((item), NULL);
#define STOP_THREAD(item) while(pthread_cancel ( (item) )!= 0){;}   pthread_join((item), NULL);

#define FUN_LOCK(T) int lock ## T (T *item) {if (item == NULL) {return 0;} if (pthread_mutex_lock(&(item->mutex.self)) != 0) {return 0;}return 1;}
#define FUN_TRYLOCK(T) int tryLock ## T (T  *item) {if (item == NULL) {return 0;} if (pthread_mutex_trylock(&(item->mutex.self)) != 0) {return 0;}return 1;}
#define FUN_UNLOCK(T) int unlock ## T (T *item) {if (item == NULL) {return 0;} if (pthread_mutex_unlock(&(item->mutex.self)) != 0) {return 0;}return 1;}

#define DEC_FUN_LOCK(T) extern int lock ## T (T *item);
#define DEC_FUN_TRYLOCK(T) extern int tryLock ## T (T  *item);
#define DEC_FUN_UNLOCK(T) extern int unlock ## T (T *item);

enum {
    APP_INIT = 90,
    APP_INIT_DATA,
    APP_RUN,
    APP_STOP,
    APP_RESET,
    APP_EXIT
} State;

typedef enum {
   OFF,
   INIT,
   RUN,
   DO,
   TERMINATED,
   SEARCH_NEED,
   SEARCH_PATH,
   FIND_CHANNELS,
   SLAVE_GET,
   SLAVE_RESET,
   SLAVE_COOP,
   BUSY,
   IDLE,
   WAIT,
   OPENED,
   CLOSED,
   OPEN,
   CLOSE,
   UNDEFINED,
   DISABLE,
   FAILURE
} ProgState;



typedef struct {
    char *buf;
    size_t *s1_offset;
    size_t buf_length;
    size_t buf_max_length;
    size_t length;
    size_t max_length;
} S1BList;
#define BLIST_ITEM(list,iname, ind)  ((list)->buf+(list)->iname##_offset[ind])
#define S1BLIST_INITIALIZER {.buf=NULL, .s1_offset=NULL, .buf_length=0, .buf_max_length=0, .length=0, .max_length=0}
#define FREE_S1BLIST(list) free((list)->s1_offset); free((list)->buf);  (list)->buf=NULL; (list)->s1_offset=NULL; (list)->buf_length=0;(list)->buf_max_length=0;(list)->length=0; (list)->max_length=0;
#define NULL_S1BLIST(list) memset((list)->buf,0,(list)->buf_max_length * sizeof (*(list)->buf));memset((list)->s1_offset,0,(list)->max_length * sizeof (*(list)->s1_offset));(list)->buf_length=0;(list)->length=0;

extern int s1blist_push(size_t min_buf_alloc_length, size_t min_item_alloc_length, S1BList *list, const char *str);

enum {
    APP_MUTEX_INIT,
    APP_MUTEX_CREATED,
    APP_MUTEX_INITIALIZED
} AppMutexState;

typedef struct {
    pthread_mutex_t self;
    pthread_mutexattr_t attr;
    int state;
} Mutex;
#define MUTEX_INITIALIZER {.self=PTHREAD_MUTEX_INITIALIZER, .state = APP_MUTEX_INIT}
//#define MUTEX pthread_mutex_t
//#define DEF_MUTEX(V) pthread_mutex_t V = PTHREAD_MUTEX_INITIALIZER;
//#define IF_LOCK_MUTEX(P) if(pthread_mutex_lock(P) != 0)
//#define IF_TRYLOCK_MUTEX(P) if(pthread_mutex_trylock(P) != 0)
//#define UNLOCK_MUTEX(P) pthread_mutex_unlock(P)

struct channel_ts_st {
    int id;
    void * data;
    int save;
    uint32_t error_code;
    
    int sock_fd;
    struct timespec cycle_duration;
    pthread_t thread;
    Mutex mutex;
    struct channel_ts_st *next;
};


extern char * strcpyma(char **dest, char *src);

extern void putse(const char *str);

extern void printfe(const char *str, ...);

extern int file_exist(const char *filename);

extern int readConf(const char *path, char conninfo[LINE_SIZE], char app_class[NAME_SIZE]);

extern void conSig(void (*fn)());

extern void setPriorityMax(int policy);

extern int readHostName(char *hostname);

extern int initPid(int *pid_file, int *pid, const char *pid_path);

extern char * getAppState(char state);

extern void freePid(int *pid_file, int *pid, const char *pid_path);

extern int initMutex(Mutex *m);

extern void freeMutex(Mutex *m);

extern int lockMutex(Mutex *item);

extern int tryLockMutex(Mutex *item);

extern int unlockMutex(Mutex *item);

extern void skipLine(FILE* stream);

extern int createThread(pthread_t *new_thread,void *(*thread_routine) (void *),char *cmd);

extern int createMThread(pthread_t *new_thread, void *(*thread_routine) (void *), void * data);

extern int threadCancelDisable(int *old_state) ;

extern int threadSetCancelState(int state);



#endif 

