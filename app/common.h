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
#define NOIDS_CONFIG_FILE			"" CONF_DIR "noid/items.tsv"
#define NOIDS_IGET_DIR				"" CONF_DIR "noid/interface/get/interval/"
#define NOIDS_GET_DIR				"" CONF_DIR "noid/interface/get/simple/"
#define OS_DEVICE_DIR					"/dev"

#endif




