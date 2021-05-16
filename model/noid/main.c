#include "main.h"

static int step_RUN(Noid *self);
static int step_OFF(Noid *self);
static int step_FAILURE(Noid *self);
static int step_INIT(Noid *self);

void noid_resetData(Noid *self){
	mutex_lock(&self->mutex);
	FORLISTN(self->igcommands, i){
		nigc_reset(&self->igcommands.items[i]);
	}
	self->control = step_INIT;
	mutex_unlock(&self->mutex);
}

NoidGetCommand *noid_getIntervalGetCmd(Noid *self, int cmd){
	FORLISTN(self->igcommands, j){
		NoidGetCommand *command = &self->igcommands.items[j].command;
		if(cmd == command->id){
		   return command;
		}
	}
	return NULL;
}


static int step_RUN(Noid *self){
	//thread_cancelDisable();
	mutex_lock(&self->mutex);
	FORLISTN(self->igcommands, i){
		nigc_control(&self->igcommands.items[i], self->id);
	}
	mutex_unlock(&self->mutex);
	//thread_cancelEnable();
	return 1;
}

static int step_OFF(Noid *self){
	return 1;
}

static int step_FAILURE(Noid *self){
	return 1;
}

static int step_INIT(Noid *self){
	//thread_cancelDisable();
	mutex_lock(&self->mutex);
	self->control = step_RUN;
	mutex_unlock(&self->mutex);
	//thread_cancelEnable();
	return 1;
}

#ifdef MODE_DEBUG
static void cleanup_handler(void *arg) {
	Noid *self = arg;
	printf("cleaning up noid thread (id=%d)\n", self->id);
}
#endif

static void *thread_main(void *arg) {
	Noid *self = arg;
	printdo("noid thread (id=%d) has been started\n", self->id);
#ifdef MODE_DEBUG
    pthread_cleanup_push(cleanup_handler, self);
#endif
	while(1) {
		struct timespec t1 = getCurrentTime();
		CONTROL(self);
		delayTsIdleRest(self->cycle_duration, t1);
	}
#ifdef MODE_DEBUG
	pthread_cleanup_pop(1);
#endif
}

void noid_start(Noid *self){
	self->control = step_INIT;
}

const char *noid_getStateStr(Noid *self){
	if(self->control == step_RUN)			return "RUN";
	else if(self->control == step_OFF)		return "OFF";
	else if(self->control == step_FAILURE)	return "FAILURE";
	else if(self->control == step_INIT)		return "INIT";
	return "?";
}

void noid_terminate(Noid *self){
	mutex_lock(&self->mutex);
	STOP_THREAD(self->thread)
	mutex_unlock(&self->mutex);
}

void noid_free(Noid *self){
	mutex_lock(&self->mutex);
	nigcList_free(&self->igcommands);
	mutex_unlock(&self->mutex);
	mutex_free(&self->mutex);
}

int noid_setParam(Noid *self, int id, const char *iget_file, const char *iget_dir, const char *file_type){
	LIST_RESET(&self->igcommands)
	self->id = id;
	if(!nigcList_init(&self->igcommands, iget_dir, iget_file, file_type)){
		return 0;
	}
	return 1;
}

int noid_begin(Noid *self){
	self->cycle_duration = NOID_CYCLE_DURATION;
	self->control = step_INIT;
	if (!mutex_init(&self->mutex)){
		putsde("failed to initialize noid mutex\n");
		return 0;
	}
	if(!thread_create(&self->thread, thread_main, self)) {
		mutex_free(&self->mutex);
		return 0;
	}
	return 1;
}




