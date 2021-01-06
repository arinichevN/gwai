#include "main.h"

static int step_RUN(Channel *self);

static int step_OFF(Channel *self);

static int step_FAILURE(Channel *self);

static int step_INIT(Channel *self);

void channel_resetData(Channel *self){
	mutex_lock(&self->mutex);
	FORLISTN(self->igcmd_list, i){
		sigc_reset(&self->igcmd_list.item[i]);
	}
	self->control = step_INIT;
	mutex_unlock(&self->mutex);
}

SlaveGetCommand *channel_getIntervalGetCmd(Channel *self, int cmd){
	FORLISTN(self->igcmd_list, j){
		SlaveGetCommand *command = &self->igcmd_list.item[j].command;
		if(cmd == command->id){
		   return command;
		}
	}
	return NULL;
}


static int step_RUN(Channel *self){
	mutex_lock(&self->mutex);
	FORLISTN(self->igcmd_list, i){
		sigc_control(&self->igcmd_list.item[i], self->id);
	}
	mutex_unlock(&self->mutex);
	return 1;
}

static int step_OFF(Channel *self){
	return 1;
}

static int step_FAILURE(Channel *self){
	return 1;
}

static int step_INIT(Channel *self){
	mutex_lock(&self->mutex);
	self->control = step_RUN;
	mutex_unlock(&self->mutex);
	return 1;
}

#ifdef MODE_DEBUG
static void cleanup_handler(void *arg) {
	Channel *self = arg;
	printf("cleaning up channel thread (id=%d)\n", self->id);
}
#endif

static void *thread_main(void *arg) {
	Channel *self = arg;
	printdo("channel thread (id=%d) has been started\n", self->id);
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

void channel_start(Channel *self){
	self->control = step_INIT;
}

const char *channel_getStateStr(Channel *self){
	if(self->control == step_RUN)			return "RUN";
	else if(self->control == step_OFF)		return "OFF";
	else if(self->control == step_FAILURE)	return "FAILURE";
	else if(self->control == step_INIT)		return "INIT";
	return "?";
}

void channel_free(Channel *self){
	mutex_lock(&self->mutex);
	STOP_THREAD(self->thread)
	FORLISTN(self->igcmd_list, j){
		sigc_free(&self->igcmd_list.item[j]);
	}
	FREE_LIST(&self->igcmd_list);
	mutex_unlock(&self->mutex);
	mutex_free(&self->mutex);
}

int channel_setParam(Channel *self, int id, const char *iget_file, const char *iget_dir, const char *file_type){
	LIST_RESET(&self->igcmd_list)
	self->id = id;
	if(!sigcList_init(&self->igcmd_list, iget_dir, iget_file, file_type)){
		return 0;
	}
	return 1;
}

int channel_begin(Channel *self){
	self->cycle_duration = CHANNEL_CYCLE_DURATION;
	self->control = step_INIT;
	if (!mutex_init(&self->mutex)){
		putsde("failed to initialize channel mutex\n");
		return 0;
	}
	if(!thread_create(&self->thread, thread_main, self)) {
		mutex_free(&self->mutex);
		return 0;
	}
	return 1;
}




