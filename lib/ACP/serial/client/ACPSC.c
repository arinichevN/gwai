#include "ACPSC.h"

ACPSC *acpsc_newBegin(){
	size_t sz = sizeof (ACPSC);
	ACPSC *self = malloc(sz);
	if(self == NULL) {
		putsde("failed to allocate memory for new ACPSC\n");
		return self;
	}
	memset(self, 0, sz);
	if(!acpscpLList_begin(&self->ports)){
		free(self);
		return NULL;
	}
	if(!acpscidLList_begin(&self->ids)){
		acpscpLList_free(&self->ports);
		free(self);
		return NULL;
	}
	self->id_mapper = acpscim_newBegin(&self->ports, &self->ids);
	if(self->id_mapper == NULL){
		acpscidLList_free(&self->ids);
		acpscpLList_free(&self->ports);
		free(self);
		return NULL;
	}
	return self;
}

int acpsc_createNewPort(ACPSC *self, const char *serial_file_name, int serial_rate, const char *serial_config) {
	ACPSCPort *new_port = acpscp_newBegin(serial_file_name, serial_rate, serial_config);
	if(new_port == NULL) {
		return 0;
	}
	if(!acpscpLList_add(&self->ports, new_port, ACPSC_MAX_PORT_COUNT)) {
		acpscp_free(new_port);
		return 0;
	}
	acpscim_connectToPort(self->id_mapper, new_port);
	return 1;
}

int acpsc_addRemoteID(ACPSC *self, int id) {
	ACPSCID *new_id = acpscid_newBegin(id);
	if(new_id == NULL) {
		return 0;
	}
	if(!acpscidLList_add(&self->ids, new_id, ACPSC_MAX_ID_COUNT)) {
		acpscid_free(new_id);
		return 0;
	}
	return 1;
}

void acpsc_free(ACPSC *self) {
	if (self == NULL) return;
	acpscim_free(self->id_mapper);
	acpscpLList_free(&self->ports);
	acpscidLList_free(&self->ids);
	free(self);
}

int acpsc_sendRequestToRemoteID(ACPSC *self, int remote_id, const char *request_str){
	FOREACH_LLIST(id, &self->ids, ACPSCID){
		if(remote_id == id->value){
			acpscid_lock(id);
			if(acpscid_isOwned(id)){
				ACPSCPort *port = (ACPSCPort *) id->owner;
				acpscp_lock(port);
				int r = acpscp_send(port, request_str);
				acpscp_unlock(port);
				acpscid_unlock(id);
				return r;
			}
			printde("unconnected id=%d\n", id->value);
			acpscid_unlock(id);
			return ACP_ERROR_CONNECTION;
		}
	}
	//try to find id?
	return ACP_NOT_FOUND;
}

int acpsc_getFromRemoteID(ACPSC *self, int remote_id, const char *request_str, char *response, size_t response_length){
	FOREACH_LLIST(id, &self->ids, ACPSCID){
		if(remote_id == id->value){
			acpscid_lock(id);
			if(acpscid_isOwned(id)){
				ACPSCPort *port = (ACPSCPort *) id->owner;
				acpscp_lock(port);
				int r = acpscp_send(port, request_str);
				if(r != ACP_SUCCESS){
					printde("failed to send request id=%d\n", remote_id);
					acpscp_unlock(port);
					acpscid_unlock(id);
					return r;
				}
				NANOSLEEP(0, 100000);
				r = acpscp_readResponse(port, response, response_length);
				acpscp_unlock(port);
				if(r == ACP_ERROR_NO_RESPONSE){
					id->state = ACPSCID_NOT_FOUND;
					acpscim_IDLost(self->id_mapper, id);
				}
				acpscid_unlock(id);
				
				if(r != ACP_SUCCESS){
					printde("failed to read response where id=%d\n", remote_id);
					return r;
				}
				return r;
			}
			printde("unconnected id=%d\n", id->value);
			mutex_unlock(&id->mutex);
			return ACP_ERROR_CONNECTION;
		}
	}
	//try to find id?
	return ACP_NOT_FOUND;
}

int acpsc_sendRequestBroadcast(ACPSC *self, const char *request_str){
	int result = ACP_SUCCESS;
	FOREACH_LLIST(port, &self->ports, ACPSCPort){
		acpscp_lock(port);
		int r = acpscp_send(port, request_str);
		acpscp_unlock(port);
		if(r != ACP_SUCCESS){
			result = ACP_ERROR;
		}
	}
	return result;
}

//**********************************************************************************************************************
int acpsc_getBroadcast(ACPSC *self, const char *request_str, char *response, size_t response_length){
	int result = ACP_ERROR;
	FOREACH_LLIST(port, &self->ports, ACPSCPort){
		acpscp_lock(port);
		int r = acpscp_send(port, request_str);
		if(r != ACP_SUCCESS){
			putsde("failed to send request\n");
			acpscp_unlock(port);
			goto next;
		}
		NANOSLEEP(0, 100000);
		r = acpscp_readResponse(port, response, response_length);
		acpscp_unlock(port);
		if(r == ACP_SUCCESS){
			return r;
		}
		next:
		;
	}
	return result;
}
