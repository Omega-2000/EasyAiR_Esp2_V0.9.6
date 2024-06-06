  idf_component_register(SRCS "Updater.c"
                    INCLUDE_DIRS "." "include"
                    REQUIRES TWAI CAN_file_transfer)

include_directories($ENV{IDF_PATH}/components/driver/include)

include_directories($ENV{IDF_PATH}/components/spiffs/include)
include_directories($ENV{IDF_PATH}/components/app_update/include)  
include_directories($ENV{IDF_PATH}/components/esp_partition/include) #esp_partition.h
include_directories($ENV{IDF_PATH}/components/bootloader_support/include)

#solo per Jey idf version v4.4 esp_partition.h si trova qui sotto
#include_directories($ENV{IDF_PATH}/components/spi_flash/include)
include_directories($ENV{IDF_PATH}/components/esp_app_format/include)
#include_directories($ENV{IDF_PATH}/components/driver/include)






