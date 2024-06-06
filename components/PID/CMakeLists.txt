idf_component_register(SRCS "PID.c"
                    INCLUDE_DIRS "include"
                    REQUIRES TWAI SD fatfs)

include_directories($ENV{IDF_PATH}/components/driver/include)
include_directories($ENV{IDF_PATH}/components/fatfs/vfs)
include_directories($ENV{IDF_PATH}/components/fatfs/src)
include_directories($ENV{IDF_PATH}/components/wear_levelling/include)
include_directories($ENV{IDF_PATH}/components/esp_partition/include)
include_directories($ENV{IDF_PATH}/components/sdmmc/include)