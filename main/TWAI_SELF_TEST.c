#include <stdio.h>
#include <stdlib.h>//
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"//
#include "driver/twai.h"
#include "C:\Users\alessandro\esp\esp-idf\components\esp_timer\include\esp_timer.h"  //  esp_timer_get_time() --> micros --> / 1000  --> millis

#define TX_GPIO_NUM             17
#define RX_GPIO_NUM             18
#define LEN                     8
#define MSG_ID                  0x888   //11 bit standard format ID
#define SELF_TEST_TAG           "_Twai_Self_Test_"

twai_message_t tx_msg = {.data_length_code = 1, .identifier = MSG_ID, .self = 1};
twai_message_t rx_message;
uint8_t data[LEN] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
unsigned long t = 0;
int cont = 0;
int cont_invio = 0;
esp_err_t init;

esp_err_t TWAI_self_test() {
    static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    static const twai_filter_config_t f_config = { .acceptance_code = (0x8 << 21) | (0x7 << 21), .acceptance_mask = ~(0xF << 21), .single_filter = false };
    static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NO_ACK);

    //Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) init = ESP_OK; else init = ESP_FAIL;

    //Start TWAI driver
    if (twai_start() == ESP_OK) init = ESP_OK; else init = ESP_FAIL;

    // Reconfigure alerts to detect frame receive, Bus-Off error and RX queue full states
    uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL;
    if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) init = ESP_OK; else init = ESP_FAIL;

    //  ----------------------------------------------      ^ setup           v loop

    if (init == ESP_OK) {
        while (cont < 3) {
            if(twai_receive(&rx_message, 0) == ESP_OK) {  //  se ricevo un messaggio 
                //ESP_LOGI(SELF_TEST_TAG, "Msg received - Data = %d", rx_message.data[0]);
                cont ++;
            }

            if ( ((esp_timer_get_time()/1000) - t) > 10) {
                tx_msg.data[0] = data[7];
                if (twai_transmit(&tx_msg, 0) != ESP_OK) {
                    ESP_LOGE(SELF_TEST_TAG, "Twai Self Test --> FAIL (invio non riuscito)\n");
                    return ESP_FAIL;
                }
                cont_invio ++;

                t = esp_timer_get_time()/1000;
            }

            if /*((cont < 3) && */(cont_invio > 10)/*)*/ {
                ESP_LOGE(SELF_TEST_TAG, "Twai Self Test --> FAIL (ricezione non avvenuta)\n");
                return ESP_FAIL;
            }

            vTaskDelay(pdMS_TO_TICKS(10));
        }
        twai_stop();
        twai_driver_uninstall();

        ESP_LOGI(SELF_TEST_TAG, "Twai Self Test --> OK\n");
        return ESP_OK;
    } else {
        ESP_LOGE(SELF_TEST_TAG, "Twai Self Test --> FAIL (setup non riuscito)\n");
        return ESP_FAIL;
    }
}
