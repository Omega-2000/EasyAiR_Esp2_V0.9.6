idf_component_register(SRCS "ELETTROVALVOLE.c"
                    INCLUDE_DIRS "include")
                    #REQUIRES MOTOR_DRIVER TWAI

include_directories($ENV{IDF_PATH}/components/esp_timer/include)

include_directories($ENV{IDF_PATH}/components/driver/include)
