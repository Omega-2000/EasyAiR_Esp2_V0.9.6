#include "VARIABLES_INIT.c"

//  INIZIALIZZAZIONE MACCHINA
void inizializzazione() {
    //  ELETTROVALVOLE
    ELETTROVALVOLE_init();
    //  PEDALE
    PEDALE_init();
    //  TWAI SELF TEST
    if (TWAI_self_test() != ESP_OK) return;
    //  TWAI
    if (TWAI_init() == ESP_OK) ESP_LOGI(INIT_TAG, "Twai initialization --> OK\n"); else {ESP_LOGE(INIT_TAG, "Twai initialization --> FAIL\n"); return;}
    //  I2C
    if (PID_i2c_master_init() == ESP_OK) {ESP_LOGI(INIT_TAG, "I2C initialization --> OK\n"); I2c_works = 1;} else {ESP_LOGE(INIT_TAG, "I2C initialization --> FAIL\n"); I2c_works = 0; return;}
    //  PID
    PID_begin(&PID, GPIO_NUM_16, kp, ki, kd); ESP_LOGI(INIT_TAG, "Pid initialization --> OK\n");
    //  SPIFF
    if (SPIFFS_init(&SPIFFS) == ESP_OK) {ESP_LOGI(INIT_TAG, "SPIFF initialization --> OK\n"); Spiff_works = 1;} else {ESP_LOGE(INIT_TAG, "SPIFF initialization --> FAIL\n"); Spiff_works = 0;}
    //  SD
    if (SD_init(&SD, PIN_NUM_MISO, PIN_NUM_MOSI, PIN_NUM_CLK, PIN_NUM_CS) == ESP_OK) {ESP_LOGI(INIT_TAG, "SD initialization --> OK\n"); SD_works = 1;} else {ESP_LOGE(INIT_TAG, "SD initialization --> FAIL\n"); SD_works = 0;}
    SD_listDir(&SD, MOUNT_POINT);

    //if (!Spiff_works && !SD_works) { TWAI_sendError(0x0B); return; }        //  se non funziona nè SPIFF nè SD o anche se non funziona solo SPIFF -> NO MACCHINA PRONTA

    ESP_LOGI(INIT_TAG, "\nIniziallizazione END ------------------------------------------------------------------------------------------------------------------------------\n\n");
}


//  MAIN CONTROLLER
void taskMain(void *pvParameters) {

    EM_configure_gpio_interrupt();              //  INTERRUPT EMERGENZE
    //  TEST FOTOCELLULA
    if (EM_test_fotocellule() == true) {ESP_LOGI(INIT_TAG, "Fotocellula test --> OK\n"); Fotocellula_works = 1;} else {ESP_LOGE(INIT_TAG, "Fotocellula test --> FAIL\n"); Fotocellula_works = 0;}

    //  AVVIO DEI 3 ESP = PING + VALORI INIZIALI
    delay(3000);
    TWAI_pingRequest(ESP1_D_ID);
    TWAI_pingRequest(ESP3_D_ID);                //  invio richiesta di ping e attendere le due risposte da esp1 e esp3
    //t_pingRequest = millis();
    macchina_ready();

    //  MACCHINA PRONTA --> AVVIO FUNZIONALITA' PROGRAMMA
    //EM_configure_gpio_interrupt();              //  INTERRUPT EMERGENZE
    t_init();                                   //  fai partire i timer effettivamente quando la macchina è pronta
    PID_setPidParameters(&PID, kp, ki, kd);     //  imposta i parametri per il pid all'avvio (dopo averli letti in SD)
    PID_setOffset(&PID, offset);
    //PID_send_temperatura_iniziale(&PID);

    while (1) {
        if (!perform_update) {
            MSG_message_receiver();                 //  MESSAGE RECEIVER
            EM_loop();                              //  EMERGENZE
            PEDALE_loop();                          //  PEDALE
            ELETTROVALVOLE_off();                   //  ELETTROVALVOLE off
            MSG_update_value_in_SD();               //  AGGIORNA VALORI IN SD

            //  ORE DI UTILIZZO IN SPIFF
            MSG_lettura_in_SPIFF();
            SPIFF_update_statistics(&SPIFFS);

            /* // PER TEST FOTOCELLULE
            if ((millis() - t_print) >= 1000) {
                ESP_LOGI(FOTOC_TAG, "FOTOCELLULA 1 --> %d", gpio_get_level(FOTOC1));
                ESP_LOGI(FOTOC_TAG, "FOTOCELLULA 2 --> %d", gpio_get_level(FOTOC2));
                t_print = millis();
            }
            */

        } else vTaskSuspend(globalTaskHandler);

        delay(10);
    }
}


//  PID CONTROLLER
void taskPid(void *pvParameters) {

    while (1) {
        if (!perform_update) {
            if (macchina_pronta) {
                //  WATCHDOG
                if (!watchdog_init) {
                    ESP_ERROR_CHECK(esp_task_wdt_deinit());
                    ESP_LOGI("RTC WATCHDOG", "deinitialized");
                    PID_init_WDT();
                    watchdog_init = 1;

                    PID_avvio_in_SD();
                }

                /*if (!allow_pid) {
                    PID_stop(&PID);
                    PID_scrivi_file_SD("PID OFF");
                }*/
                if (((millis() - last_pid) >= 200) || PID_1_st_ch) {
                    PID_task(&PID);
                    last_pid = millis();
                }

                if (((millis() - t_temperatura_attuale) > 1000) || PID_1_st_ch) {
                    PID_1_st_ch = 0;
                    PID_temperatura_attuale_loop(&PID);         //  INVIO TEMPERATURA (TWAI)  &  SALVA TEMPERATURA (SD)
                    t_temperatura_attuale = millis();

                    //  TEST EMERGENZE --> FOTOCELLULA2
                    //ESP_LOGI(EMERGENZE_TAG, "EMERGENZE:%d - FOTOC1:%d - FOTOC2:%d - FUNGO1:%d - FUNGO2:%d - RESET:%d", gpio_get_level(EMERGENZE), gpio_get_level(FOTOC1), gpio_get_level(FOTOC2), gpio_get_level(EM1), gpio_get_level(EM2), gpio_get_level(RESET_EM));
                }

                //  PID_temp_in_SD();                       //  SALVA TEMPERATURA (SD)

                if (pid_restart == 1) {                     //  RESTART PID (RESET)
                    PID_start(&PID, target_temperature);
                    PID_temperatura_attuale_loop(&PID);
                    pid_restart = 0;
                    
                    ESP_LOGI("taskPid", "pid --> ON");
                    //PID_scrivi_file_SD("PID ON");
                    SD_operation(&SD, 1, 1, "PID ON");
                }

                /*if (init_or_deinit == 1) {
                    if ((millis() - t_watchdog) > 50) {
                        PID_init_WDT();
                        init_or_deinit = 0;
                        t_watchdog = millis();
                    }
                }
                if (init_or_deinit == 2) {
                    if ((millis() - t_watchdog) > 50) {
                        PID_deinit_WDT();
                        init_or_deinit = 0;
                        t_watchdog = millis();
                    }
                }*/
            }
        } else vTaskSuspend(globalTaskHandler_2);

        delay(10);
    }
}


void taskUpdate(void *pvParameters) {
    while (1) {
        while (!update_finished) {
            delay(10);
            if (perform_update) {
                bool res = performUpdate();
                ESP_LOGI("taskUpdate", "performUpdate comnfirmed");
                update_finished = 1;
            }
        }

        if(TWAI_get() == ESP_OK) {
            if (message.identifier == RESTART_PROCESSOR)
            {
                ESP_LOGI("taskUpdate", " RESTART ESP...");
                vTaskDelay(10);
                esp_restart();
            }
        }

        delay(10);
    }
}


void app_main(void)
{
    ESP_LOGI("FW ", " EasyAiR_Esp2_V0.9.6");    //  [V0.9.5] + [RESTART SUL CICLO D'AVVIO] + [OFFSET DI -2] + [GESTIONE NUOVE EMERGENZE CON ENABLE E POSSIBILITA' DI ABILITARE FOTOCELLULA N.RO 2 (istruzioni in EMERGENZE.h)]
    ESP_LOGI("FW ", " Version: %d", versioneFW);

    inizializzazione();

    xTaskCreatePinnedToCore(
        taskMain,               // Task function
        "MAIN CONTROLLER",      // Task name
        3072*2,                 // Stack size (in words, not bytes)         //  prima era 8192*2            3072*2
        NULL,                   // Task parameter
        configMAX_PRIORITIES-1 /*9*//*1*/,                      // Task priority
        globalTaskHandler,      // Task handle
        /*1*/0                       // Core ID
    );

    xTaskCreatePinnedToCore(
        taskPid,
        "PID CONTROLLER",
        3072,
        NULL,
        configMAX_PRIORITIES-1/*2*/,
        globalTaskHandler_2,
        /*0*/1
    );

    xTaskCreate(
        taskUpdate,
        "UPDATE TASK",
        4096,
        NULL,
        configMAX_PRIORITIES-1/*2*/,
        globalTaskHandler_3
    );

}
