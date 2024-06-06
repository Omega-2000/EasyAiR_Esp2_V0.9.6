idf_component_register(SRCS "CONFIG_FILE.c"
                    INCLUDE_DIRS "include"
                    REQUIRES PID)
# REQUIRES json
include_directories($ENV{IDF_PATH}/components/spiffs/include)

include_directories($ENV{IDF_PATH}/components/driver/include)

include_directories($ENV{IDF_PATH}/components/driver/deprecated)
