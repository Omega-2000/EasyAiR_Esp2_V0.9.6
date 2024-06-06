#include "ELETTROVALVOLE.h"

void ELETTROVALVOLE_init() {
    gpio_set_direction(ALZA, GPIO_MODE_OUTPUT);
    gpio_set_direction(ABBASSA, GPIO_MODE_OUTPUT);
}

void ELETTROVALVOLE_off() {
    //if (fotocellula_interrotta == 1) {}
    if ((abbassa == 1) && (((esp_timer_get_time()/1000) - t_abbassa) > 1000)) {
        gpio_set_level(ABBASSA, 0);
        ESP_LOGI(ELETTROVALVOLE_TAG, "Abbassa OFF");
        t_abbassa_off = esp_timer_get_time()/1000;
        abbassa = 0;
    }

    if ((alza == 1) && (((esp_timer_get_time()/1000) - t_alza) > 1000)) {
        gpio_set_level(ALZA, 0);
        ESP_LOGI(ELETTROVALVOLE_TAG, "Alza OFF");
        t_alza_off = esp_timer_get_time()/1000;
        alza = 0;
    }
}
