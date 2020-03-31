#ifndef MODEL_SLAVE_SET_COMMAND_H
#define MODEL_SLAVE_SET_COMMAND_H

#define CHANNELS_SET_DIR "" CONF_DIR "channel/interface/set/"

typedef struct {
    char name[SLAVE_CMD_MAX_LENGTH];
} SlaveSetCommand;
DEC_LIST(SlaveSetCommand)

extern int sscList_init(SlaveSetCommandList *list, const char *dir, const char *file_name, const char *file_type);

#endif 
