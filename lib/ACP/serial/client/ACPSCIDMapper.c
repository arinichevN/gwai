#include "ACPSCIDMapper.h"

static void step_SEARCH_UNCONNECTED(ACPSCIDMapper *self);

static void step_SEARCH_ALL(ACPSCIDMapper *self);

static int setIDOwner(ACPSCIDMapper *self, int id, void *owner){
	FOREACH_LLIST(item, self->ids, ACPSCID){
		if(item->value == id){
			acpscid_lock(item);
			acpscid_setOwner(item, owner);
			printdo("id has been found %d\n", id);
			acpscid_unlock(item);
			return 1;
		}
	}
	return 0;
}

static void onCheckRemoteIDExistsDone(void *slave, int id, int result, void *port){
	ACPSCIDMapper *self = (ACPSCIDMapper *) slave;
	switch(result){
		case ACP_SUCCESS:
			setIDOwner(self, id, port);
			return;
	}
	printdo("id not found %d\n", id);
}

static void onPortConnected(void *vself, void *vport){
	;
}

static void onPortDisconnected(void *vself, void *vport){
	ACPSCIDMapper *self = (ACPSCIDMapper *) vself;
	ACPSCPort *port = (ACPSCPort *) vport;
	FOREACH_LLIST(id, self->ids, ACPSCID){
		acpscid_lock(id);
		if(id->owner == port){
			id->state = ACPSCID_PORT_DISCONNECTED;
		}
		acpscid_unlock(id);
	}
}

void acpscim_IDLost(ACPSCIDMapper *self, ACPSCID *id){
	acpscid_lock(id);
	id->state = ACPSCID_PORT_DISCONNECTED;
	acpscid_unlock(id);
	mutex_lock(&self->mutex);
	self->cycle_duration = ACPSCIDM_CYCLE_SEARCH_UNCONNECTED;
	self->control = step_SEARCH_UNCONNECTED;
	mutex_unlock(&self->mutex);
}

static void step_SEARCH_UNCONNECTED(ACPSCIDMapper *self){
	FOREACH_LLIST(id, self->ids, ACPSCID){
		thread_cancelDisable();
		acpscid_lock(id);
		int owned = acpscid_isOwned(id);
		acpscid_unlock(id);
		if(!owned){
			FOREACH_LLIST(port, self->ports, ACPSCPort){
				acpscp_checkRemoteIDExists(port, id->value);
			}
			thread_cancelEnable();
			NANOSLEEP(0, 100000000);
			thread_cancelDisable();
		}
		thread_cancelEnable();
	}
}

static void step_SEARCH_ALL(ACPSCIDMapper *self){
	mutex_lock(&self->mutex);
	FOREACH_LLIST(id, self->ids, ACPSCID){
		thread_cancelDisable();
		FOREACH_LLIST(port, self->ports, ACPSCPort){
			acpscp_checkRemoteIDExists(port, id->value);
		}
		thread_cancelEnable();
		NANOSLEEP(0, 100000);
	}
	self->cycle_duration = ACPSCIDM_CYCLE_SEARCH_UNCONNECTED;
	self->control = step_SEARCH_UNCONNECTED;
	mutex_unlock(&self->mutex);
}

#ifdef MODE_DEBUG
static void cleanup_handler(void *arg) {
	puts("cleaning up ID mapper thread\n");
}
#endif

static void *thread_main(void *arg) {
	ACPSCIDMapper *self = arg;
	putsdo("ID mapper thread has been started\n");
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

ACPSCIDMapper *acpscim_new() {
	size_t sz = sizeof (ACPSCIDMapper);
	ACPSCIDMapper *self = malloc(sz);
	if(self == NULL) {
		putsde("failed to allocate memory for new ACPSCIDMapper\n");
		return self;
	}
	memset(self, 0, sz);
	return self;
}

ACPSCIDMapper *acpscim_newBegin(ACPSCPortLListm *ports, ACPSCIDLListm *ids){
	ACPSCIDMapper *self = acpscim_new();
	if(self == NULL){
		return NULL;
	}
	self->ids = ids;
	self->ports = ports;
	self->cycle_duration = ACPSCIDM_CYCLE_SEARCH_ALL;
	self->control = step_SEARCH_ALL;
	if(!mutex_init(&self->mutex)) {
		free(self);
		return NULL;
	}
	if(!thread_create(&self->thread, thread_main, self)) {
		mutex_free(&self->mutex);
		free(self);
		return NULL;
	}
	return self;
}

void acpscim_connectToPort(ACPSCIDMapper *self, ACPSCPort *port) {
	port->mapper = self;
	port->onConnected = onPortConnected;
	port->onDisconnected = onPortDisconnected;
	port->onCheckRemoteIDExistsDone = onCheckRemoteIDExistsDone;
}

void acpscim_free(ACPSCIDMapper *self) {
	if(self == NULL) return;
	mutex_lock(&self->mutex);
	STOP_THREAD(self->thread)
	mutex_unlock(&self->mutex);
	mutex_free(&self->mutex);
	free(self);
}
