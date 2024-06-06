idf_component_register(SRCS "PEDALE.c"
                    INCLUDE_DIRS "include"
                    REQUIRES TWAI EMERGENZE ELETTROVALVOLE)

include_directories($ENV{IDF_PATH}/components/driver/include)

include_directories($ENV{IDF_PATH}/components/esp_timer/include)

include_directories($ENV{IDF_PATH}/components/driver/deprecated)
