#include "main.h"

static void step_IDLE(AcptsConnection *self);
static void step_BUSY(AcptsConnection *self);

const char *acptsconn_getStateStr(AcptsConnection *self){
	if(self->control == step_IDLE)			return "IDLE";
	else if(self->control == step_BUSY)		return "BUSY";
	return "?";
}

int acptsconn_wakeUp(AcptsConnection *self, int fd){
	if(mutex_tryLock(&self->mutex)) {
		if(self->control == step_IDLE){
			self->fd = fd;
			//printdo("SERVER THREAD %zu MARKED TO START\n", self->id);
			self->control = step_BUSY;
			mutex_unlock(&self->mutex);
			return 1;
		}
		mutex_unlock(&self->mutex);
		return 0;
	}
	//printdo("failed to lock mutex %zu\n", self->id);
	return 0;
}

static void step_IDLE(AcptsConnection *self){
	NANOSLEEP(0,1000000);
}

static void step_BUSY(AcptsConnection *self){
	//printdo("SERVERM_BUSY %zu\n", self->id);
	//putsdo("\n\n");
	//printdo("connection is busy %zu\n", self->id);
	//thread_cancelDisable();
	mutex_lock(&self->mutex);
	int fd = self->fd;
	while (1){
		char buf[ACP_BUF_MAX_LENGTH];
		if(!acptcp_readPack(fd, buf, ACP_BUF_MAX_LENGTH)) {
			goto connection_stop;
		}
		if(!self->serve_function(fd, buf)){
			goto connection_stop;
		}
	}
	connection_stop: close(self->fd); self->control=step_IDLE; mutex_unlock(&self->mutex);
	//thread_cancelEnable();
}

static void *thread_main(void *arg){
	AcptsConnection *self = arg;
	//printf("hello from server connection service thread %d\n", self->id);
	//fflush(stdout);
	while(1){
		CONTROL(self);
	}
	return NULL;
}

AcptsConnection *acptsconn_new(int id, AcptsServeFunction serve_function){
	size_t sz = sizeof (AcptsConnection);
	AcptsConnection *self = malloc(sz);
	if(self == NULL) {
		putsde("failed to allocate memory\n");
		return NULL;
	}
	memset(self, 0, sz);
	self->id = id;
	self->control = step_IDLE;
	self->serve_function = serve_function;
	self->next = NULL;
	if(!mutex_init(&self->mutex)) {
		putsde("failed to initialize mutex");
		free(self);
		return NULL;
	}
	if(!thread_create(&self->thread, thread_main, self)) {
		putsde("failed to create thread\n");
		mutex_free(&self->mutex);
		free(self);
		return NULL;
	}
	return self;
}

void acptsconn_terminate(AcptsConnection *self){
	mutex_lock(&self->mutex);
	STOP_THREAD(self->thread)
	mutex_unlock(&self->mutex);
}

void acptsconn_free(AcptsConnection *self){
	mutex_free(&self->mutex);
	free(self);
}


