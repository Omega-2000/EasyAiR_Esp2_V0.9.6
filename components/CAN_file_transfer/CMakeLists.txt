  idf_component_register(SRCS "CAN_file_transfer.c"
                    INCLUDE_DIRS "." "include"
                    REQUIRES TWAI)
include_directories($ENV{IDF_PATH}/components/spiffs/include)
include_directories($ENV{IDF_PATH}/components/driver/include)
                    



