#include "main.h"

Acpsc *acpsc_newBegin(){
	size_t sz = sizeof (Acpsc);
	Acpsc *self = malloc(sz);
	if(self == NULL) {
		putsde("failed to allocate memory for new Acpsc\n");
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
	return self;
}

int acpsc_createNewPort(Acpsc *self, const char *serial_file_name, int serial_rate, const char *serial_dps) {
	AcpscPort *new_port = acpscp_newBegin(serial_file_name, serial_rate, serial_dps, &self->ids);
	if(new_port == NULL) {
		return 0;
	}
	if(!acpscpLList_add(&self->ports, new_port, ACPSC_MAX_PORT_COUNT)) {
		acpscp_free(new_port);
		return 0;
	}
	return 1;
}

int acpsc_addRemoteID(Acpsc *self, int id) {
	AcpscID *new_id = acpscid_newBegin(id);
	if(new_id == NULL) {
		return 0;
	}
	if(!acpscidLList_add(&self->ids, new_id, ACPSC_MAX_ID_COUNT)) {
		acpscid_free(new_id);
		return 0;
	}
	return 1;
}

void acpsc_terminate(Acpsc *self){
	acpscpLList_terminate(&self->ports);
}

void acpsc_free(Acpsc **pself) {
	Acpsc *self = *pself;
	if (self == NULL) return;
	acpscpLList_free(&self->ports);
	acpscidLList_free(&self->ids);
	free(self);
	*pself = NULL;
}

static void disconnectPortIDs(Acpsc *self, AcpscPort *port, AcpscID *locked_id) {
	FOREACH_LLIST(id, &self->ids, AcpscID){
		if(id == locked_id){
			if(id->owner == port){
				id->state = ACPSCID_PORT_DISCONNECTED;
			}
		} else {
			acpscid_lock(id);
			if(id->owner == port){
				id->state = ACPSCID_PORT_DISCONNECTED;
			}
			acpscid_unlock(id);
		}
	}
}

static void disconnectPortIDsAll(Acpsc *self, AcpscPort *port) {
	FOREACH_LLIST(id, &self->ids, AcpscID){
		acpscid_lock(id);
		if(id->owner == port){
			id->state = ACPSCID_PORT_DISCONNECTED;
		}
		acpscid_unlock(id);
	}
}

int acpsc_sendRequestToRemoteID(Acpsc *self, int remote_id, const char *request_str){
	FOREACH_LLIST(id, &self->ids, AcpscID){
		if(remote_id == id->value){
			acpscid_lock(id);
			if(acpscid_isOnline(id)){
				AcpscPort *port = (AcpscPort *) id->owner;
				acpscp_lock(port);
				int r = acpscp_send(port, request_str);
				acpscp_unlock(port);
				if(r == ACP_ERROR_DEVICE_DISCONNECTED){
					disconnectPortIDs(self, port, id);
				}
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

int acpsc_getFromRemoteID(Acpsc *self, int remote_id, const char *request_str, char *response, size_t response_length){
	FOREACH_LLIST(id, &self->ids, AcpscID){
		if(remote_id == id->value){
			acpscid_lock(id);
			if(acpscid_isOnline(id)){
				AcpscPort *port = (AcpscPort *) id->owner;
				acpscp_lock(port);
				int r = acpscp_send(port, request_str);
				if(r != ACP_SUCCESS){
					printde("failed to send request id=%d\n", remote_id);
					if(r == ACP_ERROR_DEVICE_DISCONNECTED){
						disconnectPortIDs(self, port, id);
					}
					acpscp_unlock(port);
					acpscid_unlock(id);
					return r;
				}
				NANOSLEEP(0, 100000);
				r = acpscp_readResponse(port, response, response_length);
				if(r != ACP_SUCCESS){
					printde("failed to read response where id=%d\n", remote_id);
				}
				if(r == ACP_ERROR_DEVICE_DISCONNECTED){
					disconnectPortIDs(self, port, id);
				}
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

int acpsc_sendRequestBroadcast(Acpsc *self, const char *request_str){
	int result = ACP_SUCCESS;
	FOREACH_LLIST(port, &self->ports, AcpscPort){
		acpscp_lock(port);
		int r = acpscp_send(port, request_str);
		if(r == ACP_ERROR_DEVICE_DISCONNECTED){
			disconnectPortIDsAll(self, port);
		}
		acpscp_unlock(port);
		if(r != ACP_SUCCESS){
			result = ACP_ERROR;
		}
	}
	return result;
}

//**********************************************************************************************************************
int acpsc_getBroadcast(Acpsc *self, const char *request_str, char *response, size_t response_length){
	int result = ACP_ERROR;
	FOREACH_LLIST(port, &self->ports, AcpscPort){
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
