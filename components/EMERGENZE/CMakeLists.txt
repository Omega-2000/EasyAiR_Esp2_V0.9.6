idf_component_register(SRCS "EMERGENZE.c"
                    INCLUDE_DIRS "include"
                    REQUIRES TWAI PID ELETTROVALVOLE)

include_directories($ENV{IDF_PATH}/components/esp_timer/include)

include_directories($ENV{IDF_PATH}/components/driver/include)

include_directories($ENV{IDF_PATH}/components/driver/deprecated)

#PID
include_directories($ENV{IDF_PATH}/components/fatfs/vfs)
include_directories($ENV{IDF_PATH}/components/fatfs/src)
include_directories($ENV{IDF_PATH}/components/wear_levelling/include)
include_directories($ENV{IDF_PATH}/components/esp_partition/include)
include_directories($ENV{IDF_PATH}/components/sdmmc/include)
