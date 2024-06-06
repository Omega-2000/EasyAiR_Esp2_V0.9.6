idf_component_register(SRCS "TWAI.c"
                    INCLUDE_DIRS "include")

include_directories($ENV{IDF_PATH}/components/driver/include)

include_directories($ENV{IDF_PATH}/components/spiffs/include)
include_directories($ENV{IDF_PATH}/components/app_update/include)  
include_directories($ENV{IDF_PATH}/components/esp_partition/include) #esp_partition.h
include_directories($ENV{IDF_PATH}/components/bootloader_support/include)
include_directories($ENV{IDF_PATH}/components/esp_app_format/include)
