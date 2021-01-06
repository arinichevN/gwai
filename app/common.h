#ifndef APP_COMMON_H
#define APP_COMMON_H

#define APP_ID							5

#define APP_NAME						gwst
#define APP_NAME_STR TOSTRING(APP_NAME)

#ifdef MODE_FULL
#define CONF_DIR						"/etc/controller/" APP_NAME_STR "/config/"
#endif
#ifndef MODE_FULL
#define CONF_DIR						"./config/"
#endif
#define CONF_FILE_TYPE					".tsv"
#define CONFIG_FILE						"" CONF_DIR "app.tsv"
#define SERIAL_PORTS_CONFIG_FILE		"" CONF_DIR "serial.tsv"
#define CHANNELS_CONFIG_FILE			"" CONF_DIR "channel/items.tsv"
#define CHANNELS_IGET_DIR				"" CONF_DIR "channel/interface/get/interval/"
#define CHANNELS_GET_DIR				"" CONF_DIR "channel/interface/get/simple/"
#define OS_DEVICE_DIR					"/dev"

#endif




