idf_component_register(SRCS "SD_OPERATIONS.c" "MSG_LISTENER.c"
                    INCLUDE_DIRS "include"
                    REQUIRES TWAI PID EMERGENZE PEDALE ELETTROVALVOLE CONFIG_FILE SD Updater)

include_directories($ENV{IDF_PATH}/components/driver/include)

include_directories($ENV{IDF_PATH}/components/esp_timer/include)

include_directories($ENV{IDF_PATH}/components/driver/deprecated)

include_directories($ENV{IDF_PATH}/components/spiffs/include)

include_directories($ENV{IDF_PATH}/components/fatfs/vfs)

include_directories($ENV{IDF_PATH}/components/fatfs/src)

include_directories($ENV{IDF_PATH}/components/wear_levelling/include)

include_directories($ENV{IDF_PATH}/components/esp_partition/include)

include_directories($ENV{IDF_PATH}/components/sdmmc/include)



#include_directories($ENV{IDF_PATH}/components/spiffs/include)
include_directories($ENV{IDF_PATH}/components/app_update/include)  
include_directories($ENV{IDF_PATH}/components/esp_partition/include) #esp_partition.h
include_directories($ENV{IDF_PATH}/components/bootloader_support/include)
#solo per Jey idf version v4.4 esp_partition.h si trova qui sotto
include_directories($ENV{IDF_PATH}/components/spi_flash/include)
include_directories($ENV{IDF_PATH}/components/esp_app_format/include)
#include_directories($ENV{IDF_PATH}/components/driver/include)