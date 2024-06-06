#include "EMERGENZE.h"

//  TEST FOTOCELLULA
bool EM_test_fotocellule() {
    gpio_set_direction(TEST_FOTO, GPIO_MODE_OUTPUT);
    //  FOTOC1
    /*gpio_config_t gpio_config_1 = {
        .pin_bit_mask = 1ULL << FOTOC1,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    ESP_ERROR_CHECK(gpio_config(&gpio_config_1));*/

    int prima = gpio_get_level(FOTOC1);
    ESP_LOGI(FOTOC_TAG, "PRIMA --> %d", prima);
    //ESP_LOGI("TEST_FOTO (prima)", "Fotocellula 2 --> %d", gpio_get_level(FOTOC2));
    delay(100);

    if (prima == 0) {
        gpio_set_level(TEST_FOTO, 1);

        delay(100);
        int dopo = gpio_get_level(FOTOC1);
        ESP_LOGI(FOTOC_TAG, "DOPO --> %d", dopo);
        //ESP_LOGI("TEST_FOTO (dopo)", "Fotocellula 2 --> %d", gpio_get_level(FOTOC2));

        delay(100);

        gpio_set_level(TEST_FOTO, 0);

        if (prima == dopo) return false; else return true;
    } else return false;    //  se 1 --> fotocellula non collegata o non funzionante o occupata
}

//  ----------------------------------------------------------------------------------------------------------------------------------------------------------

void IRAM_ATTR EM_emergency(bool funghi) {
    emergenze = 1;

    //last_stato_motore = stato_motore;
    //last_stato_motore = 0;

    last_stato_pid = PID_getState(&PID);

    if (funghi == 1) {
        //  PID
        allow_pid = 0;
        //  last_stato_pid = PID_getState(&PID);
        //stato_pid = 0;
        PID_stop(&PID);
        //PID_scrivi_file_SD("PID OFF");
        pid_off_em = 1;

        emergenze_funghi = 1;

        em_funghi = 1;
    } else emergenze_fotocellule = 1;

    //  ABBASSA
    allow_abbassa = 0;

    //  PEDALE
    allow_pedale = 0;

    allow_photocell = 0;

    if (fotoc_dis_da_reset) {
        //  riattiva fotocellule, lasciando il tasto reset non premuto e riattivando l interrupt
        gpio_set_level(GPIO_NUM_48, 1);     //  reset
        //delay(100);
        gpio_set_direction(GPIO_NUM_48, GPIO_MODE_INPUT);
        gpio_intr_enable(GPIO_NUM_48);
        fotoc_dis_da_reset = 0;
    }
}

void IRAM_ATTR EM_fotocellula1(void* arg) {
    if (gpio_get_level(FOTOC1) == 1) {  //  fotocellula interrotta
        fotocellula_interrotta = 1;

        if (abbassa == 1) fotoc_blocc_abbassa = 1;

        if (stato_motore == 1) {
            
            if (allow_photocell == 1) {
                if ((EM_azione_1 != 1) && ((millis() - t_stop_az_1_int) > 50)) { EM_start_azione_interrupt(1); print = 1; cont_print++; EM_azione_1 = 1; t_start_az_1_int = millis(); }
            }

            if ((emergenze == 0) && (allow_photocell == 1)) {
                EM_emergency(0);
                twai_clear_transmit_queue();
                TWAI_sendEmergenze(gpio_get_level(FOTOC1), gpio_get_level(FOTOC2), gpio_get_level(EM2), gpio_get_level(EM1), 0x01/*!gpio_get_level(EMERGENZE)*/);
                print = 2; cont_print++;
            
                n_emergenza = 1;
            }

        } else if ((stato_motore == 0)/* || (stato_motore == 2) || (stato_motore == 3)*/) {     //  MODIFICA PER BLINK RESET SE FOTOC OCCUPATA IN REVERSE
            if ((emergenze == 1) && ((millis() - t_stop_az_1_int) > 50)) { EM_start_azione_interrupt(1); print = 3; EM_azione_1 = 1; t_start_az_1_int = millis(); }     //  se emergenze già scattate
            else { EM_start_azione_interrupt(0); print = 4; }
        } else if (stato_motore == 2) {     //  MODIFICA PER BLINK RESET SE FOTOC OCCUPATA IN REVERSE
                if ((EM_azione_1 != 1) && ((millis() - t_stop_az_1_int) > 50)) { EM_start_azione_interrupt(1); print = 1; cont_print++; EM_azione_1 = 1; t_start_az_1_int = millis(); }
        }

    } else {
        fotocellula_interrotta = 0;

        if /*(!force_start_az_0 && */(EM_azione == 0)/*)*/ { TWAI_sendAzioneEmergenze_interrupt(0x00, 0x00); print = 5; EM_azione = -1; }

        if ((EM_azione_1 == 1) && (EM_azione == 1) && ((millis() - t_start_az_1_int) > 50)) { 
            TWAI_sendAzioneEmergenze_interrupt(0x00, 0x01); print = 6; EM_azione = -1; EM_azione_1 = -1; t_stop_az_1_int = millis();
            //if ((t_stop_az_1_int - t_start_az_1_int) < 180) { EM_start_azione_interrupt(0); print = 7; }
        }

        t_fotocellula_rilasciata = millis();
    }
}

void IRAM_ATTR EM_fotocellula2(void* arg) {
    if (gpio_get_level(FOTOC2) == 1) {  //  fotocellula interrotta
        //if (stato_motore == 2) ESP_LOGI("FOTOCELLULA 2", "SCATTATA"); else printf("fotoc2 scattata\n");
    } else {
        //ESP_LOGE("FOTOCELLULA 2", "FINE EMERGENZA");
    }
}

void IRAM_ATTR EM_fungo1(void* arg) {   //  FUNGO_SX = EM1
    if (gpio_get_level(EM1) == 1) {
        fungo_premuto = 1;

        if (abbassa == 1) fotoc_blocc_abbassa = 1;

        if (emergenze == 0) {
            EM_emergency(1);
            twai_clear_transmit_queue();
            TWAI_sendEmergenze(gpio_get_level(FOTOC1), gpio_get_level(FOTOC2), gpio_get_level(EM2), gpio_get_level(EM1), 0x01/*!gpio_get_level(EMERGENZE)*/);
        
            n_emergenza = 2;
        }

    } else fungo_premuto = 0;
}

void IRAM_ATTR EM_fungo2(void* arg) {   //  FUNGO_DX = EM2
    if (gpio_get_level(EM2) == 1) {
        fungo_premuto = 1;

        if (abbassa == 1) fotoc_blocc_abbassa = 1;

        if (emergenze == 0) {
            EM_emergency(1);
            twai_clear_transmit_queue();
            TWAI_sendEmergenze(gpio_get_level(FOTOC1), gpio_get_level(FOTOC2), gpio_get_level(EM2), gpio_get_level(EM1), 0x01/*!gpio_get_level(EMERGENZE)*/);
        
            n_emergenza = 3;
        }

    } else fungo_premuto = 0;
}

void IRAM_ATTR EM_reset(void* arg) {
    if (gpio_get_level(RESET_EM) == 0) { // Button is pressed

        //n_emergenza = 0;

        if (mi_sono_riavviato) TWAI_sendAzioneEmergenze_interrupt(0x00, 0x00);

        reset_premuto = 1;
        t_reset_premuto = millis();

        //emergenze = 0;//
        emergenze_fotocellule = 0;//
        //  allow servizi
        allow_pid = 1;
        allow_abbassa = 1;
        allow_pedale = 1;

        //  allow emergenze
        allow_photocell = 0;

        /*if ((PID_getState(&PID) == 0) && (last_stato_pid == 1)) {
            //stato_pid = 1;
            pid_restart = 1;
        }*/
        emergenze_funghi = 0;//

        TWAI_sendReset();

    } else {    //  rilasciato il pulsante
        reset_premuto = 0;//
        t_reset_premuto = 0;
        az_reset = 0;
        
        //if (emergenze == 1) stato_motore = 0;         // non penso serva dato che esp3 manda stato_motore quando scatta l'emergenza

        //emergenze = 0;//

        allow_photocell = 1;

        //  *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

        if /*(((millis() - t_fotocellula_rilasciata) < 300) ||*/ (gpio_get_level(FOTOC1) == 1)/*)*/ {  //  fotocellula interrotta
            fotocellula_interrotta = 1;

            if (abbassa == 1) fotoc_blocc_abbassa = 1;

            if (stato_motore == 1) {
                
                if (allow_photocell == 1) {
                    if (EM_azione_1 != 1) { EM_start_azione_interrupt(1); EM_azione_1 = 1; t_start_az_1_int = millis(); }
                }

                if ((emergenze == 0) && (allow_photocell == 1)) {
                    EM_emergency(0);
                    twai_clear_transmit_queue();
                    TWAI_sendEmergenze(gpio_get_level(FOTOC1), gpio_get_level(FOTOC2), gpio_get_level(EM2), gpio_get_level(EM1), 0x01/*!gpio_get_level(EMERGENZE)*/);
                
                    n_emergenza = 1;
                }

            } else if (stato_motore == 0) {
                if (emergenze == 1) { EM_start_azione_interrupt(1); EM_azione_1 = 1; t_start_az_1_int = millis(); } else EM_start_azione_interrupt(0);
            }

        }

        //  *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

        

        //last_stato_motore = 0;
        t_reset_rilasciato = millis();
    }
}

void IRAM_ATTR EM_pedale(void* arg) {
    if (gpio_get_level(PEDALE_PIN) == 0) {
        pedale = 1;
    } else {
        pedale = 0;
    }
}

/*void IRAM_ATTR EM_emergenze(void* arg) {
    //  pin emergenze a 1 = emergenze resettate --> in messaggio invio 0

    t_pin_em = millis();    //      -->     TWAI_sendEmergenze(gpio_get_level(FOTOC1), gpio_get_level(FOTOC2), gpio_get_level(EM2), gpio_get_level(EM1), 0x00);
}*/


//  INTERRUPT EMERGENZE
void EM_configure_gpio_interrupt() {
    emergenze = 0;
    emergenze_funghi = 0;
    emergenze_fotocellule = 0;

    gpio_set_direction(EMERGENZE, GPIO_MODE_INPUT);

    gpio_set_direction(ENABLE, GPIO_MODE_OUTPUT);
    gpio_set_level(ENABLE, 0);

    //  FOTOC1
    gpio_config_t gpio_config_1 = {
        .pin_bit_mask = 1ULL << FOTOC1,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    ESP_ERROR_CHECK(gpio_config(&gpio_config_1));

    //  FOTOC2
    /*gpio_config_t gpio_config_6 = {
        .pin_bit_mask = 1ULL << FOTOC2,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    ESP_ERROR_CHECK(gpio_config(&gpio_config_6));*/

    //  FUNGO 1
    gpio_config_t gpio_config_2 = {
        .pin_bit_mask = 1ULL << EM1,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    ESP_ERROR_CHECK(gpio_config(&gpio_config_2));

    //  FUNGO 2
    gpio_config_t gpio_config_3 = {
        .pin_bit_mask = 1ULL << EM2,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    ESP_ERROR_CHECK(gpio_config(&gpio_config_3));

    //  RESET
    gpio_config_t gpio_config_4 = {
        .pin_bit_mask = 1ULL << RESET_EM,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    ESP_ERROR_CHECK(gpio_config(&gpio_config_4));

    //  PEDALE
    gpio_config_t gpio_config_5 = {
        .pin_bit_mask = 1ULL << PEDALE_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    ESP_ERROR_CHECK(gpio_config(&gpio_config_5));

    gpio_install_isr_service(0);
    gpio_isr_handler_add(FOTOC1, EM_fotocellula1, NULL);
    //gpio_isr_handler_add(FOTOC2, EM_fotocellula2, NULL);
    gpio_isr_handler_add(EM1, EM_fungo1, NULL);
    gpio_isr_handler_add(EM2, EM_fungo2, NULL);
    gpio_isr_handler_add(RESET_EM, EM_reset, NULL);
    gpio_isr_handler_add(PEDALE_PIN, EM_pedale, NULL);

    /*gpio_config_t gpio_conf;

    //  FOTOCELLULA 1
    gpio_conf.intr_type = GPIO_INTR_ANYEDGE; // Interrupt on rising and falling edges
    gpio_conf.pin_bit_mask = 1ULL << FOTOC1;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config(&gpio_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(FOTOC1, EM_fotocellula1, NULL);

    //  FUNGO 1
    gpio_conf.intr_type = GPIO_INTR_ANYEDGE; //GPIO_INTR_POSEDGE;
    gpio_conf.pin_bit_mask = 1ULL << EM1;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config(&gpio_conf);
    gpio_isr_handler_add(EM1, EM_fungo1, NULL);

    //  FUNGO 2
    gpio_conf.intr_type = GPIO_INTR_ANYEDGE; //GPIO_INTR_POSEDGE;
    gpio_conf.pin_bit_mask = 1ULL << EM2;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config(&gpio_conf);
    gpio_isr_handler_add(EM2, EM_fungo2, NULL);

    //  RESET
    gpio_conf.intr_type = GPIO_INTR_ANYEDGE;    //GPIO_INTR_NEGEDGE;
    gpio_conf.pin_bit_mask = 1ULL << RESET_EM;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&gpio_conf);
    gpio_isr_handler_add(RESET_EM, EM_reset, NULL);

    //  PEDALE
    gpio_conf.intr_type = GPIO_INTR_ANYEDGE;
    gpio_conf.pin_bit_mask = 1ULL << PEDALE_PIN;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&gpio_conf);
    gpio_isr_handler_add(PEDALE_PIN, EM_pedale, NULL);*/

    //  EMERGENZE
    /*gpio_conf.intr_type = GPIO_INTR_POSEDGE;
    gpio_conf.pin_bit_mask = 1ULL << EMERGENZE;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&gpio_conf);
    gpio_isr_handler_add(EMERGENZE, EM_emergenze, NULL);*/    
}


void EM_start_azione(int8_t azione) {
    if (EM_azione == -1) {                                  //  se non c'è nessuna azione attiva -> start azione

        TWAI_sendAzioneEmergenze(0x01, azione);
        EM_azione = azione;

    } else if (EM_azione != azione) {                       //  se c'è un'altra azione attiva (ma diversa da quella che si deve attivare) -> stop azione attuale e start azione nuova

        TWAI_sendAzioneEmergenze(0x00, EM_azione);
        if (EM_azione == 1) { EM_azione_1 = -1; /*delay(100);*/ }
        if (EM_azione == 2) { EM_azione_2 = -1; /*delay(100);*/ }

        TWAI_sendAzioneEmergenze(0x01, azione);
        EM_azione = azione;

    }   //  else if (EM_azione == azione) -->                   se l'azione attiva è uguale all'azione che si vuole attivare -> non fare niente e lasciare attiva l'azione
}

void EM_start_azione_interrupt(int8_t azione) {
    if (EM_azione == -1) {                                  //  se non c'è nessuna azione attiva -> start azione

        TWAI_sendAzioneEmergenze_interrupt(0x01, azione);
        EM_azione = azione;

    } else if (EM_azione != azione) {                       //  se c'è un'altra azione attiva (ma diversa da quella che si deve attivare) -> stop azione attuale e start azione nuova

        TWAI_sendAzioneEmergenze_interrupt(0x00, EM_azione);
        if (EM_azione == 1) { EM_azione_1 = -1; /*delay(100);*/ }
        else if (EM_azione == 2) { EM_azione_2 = -1; /*delay(100);*/ }

        TWAI_sendAzioneEmergenze_interrupt(0x01, azione);
        EM_azione = azione;

    }   //  else if (EM_azione == azione) -->                   se l'azione attiva è uguale all'azione che si vuole attivare -> non fare niente e lasciare attiva l'azione
}


void EM_loop() {
    /*if (print == 6) {
        ESP_LOGW(EMERGENZE_TAG, "%ldms", t_stop_az_1_int-t_start_az_1_int);
    }*/


    //  PRINT DEBUG --> FOTOCELLULA SCATTATA VELOCEMENTE
    if (cont_print == 2) {
        ESP_LOGW(EMERGENZE_TAG, "START -> AZIONE EMERGENZA 1");
        ESP_LOGW(EMERGENZE_TAG, "EMERGENZA FOTOCELLULA");
        print = 0;
        cont_print = 0;
    } else {
        if (print == 1) {
            ESP_LOGW(EMERGENZE_TAG, "START 1 -> AZIONE EMERGENZA 1");
            print = 0;
            cont_print = 0;
        } else if (print == 2) {
            ESP_LOGW(EMERGENZE_TAG, "EMERGENZA FOTOCELLULA (solo 2)");
            print = 0;
            cont_print = 0;
        }
    }

    switch (print) {
        case 3:
            ESP_LOGW(EMERGENZE_TAG, "START 3 -> AZIONE EMERGENZA 1");
            print = 0;
            break;

        case 4:
            ESP_LOGW(EMERGENZE_TAG, "START 4 -> AZIONE EMERGENZA 0");
            print = 0;
            break;

        case 5:
            ESP_LOGW(EMERGENZE_TAG, "STOP 5 -> AZ. EM. 0");
            print = 0;
            break;

        case 6:
            ESP_LOGW(EMERGENZE_TAG, "STOP 6 -> AZ. EM. 1");
            print = 0;
            break;

        case 7:
            print = 0;
            break;

        case 8:
            ESP_LOGW(EMERGENZE_TAG, "START 8 -> AZ. EM. 0 -> <200");
            print = 0;
            break;
    }


    //  START AZIONE 0 FORZATA (blink reset non parte se azione start e azione stop sono veloci)    -->     non funziona
    /*if ( ((print == 6) || (print == 7)) && ((t_stop_az_1_int - t_start_az_1_int) < 300)) { 
        EM_start_azione_interrupt(0);
        print = 8;
        EM_azione_1 = 1;
        EM_azione = 1;
        //t_start_az_1_int = millis();
        //force_start_az_0 = 1;
    }*/


    //  STOP AZIONE 1 SE è STATO INVIATO LO START, NO LO STOP, SONO PASSATI 50ms E LA FOTOCELLULA NON è OCCUPATA MA LIBERA
    if ( (t_start_az_1_int > t_stop_az_1_int) && (gpio_get_level(FOTOC1) == 0) ) {
        if ((EM_azione_1 == 1) && (EM_azione == 1)/* && ((millis() - t_start_az_1_int) > 50)*/) { 
            TWAI_sendAzioneEmergenze(0x00, 0x01); print = 7; EM_azione = -1; EM_azione_1 = -1; t_stop_az_1_int = millis();
            ESP_LOGI(EMERGENZE_TAG, "forzato invio stop azione 1");
        }
    }
    

    //  PER CAMBIARE POP-UP EMERGENZA ANCHE SENZA PREMERE RESET
    if ( ((n_emergenza == 1) && (gpio_get_level(FOTOC1) == 0)  &&  ((gpio_get_level(EM1) == 1) || (gpio_get_level(EM2) == 1)))     || 
         ((n_emergenza == 2) && (gpio_get_level(EM1) == 0)     &&  ((gpio_get_level(FOTOC1) == 1) || (gpio_get_level(EM2) == 1)))  || 
         ((n_emergenza == 3) && (gpio_get_level(EM2) == 0)     &&  ((gpio_get_level(EM1) == 1) || (gpio_get_level(FOTOC1) == 1)))     ) {
        
        /*if ((n_emergenza == 1) && (gpio_get_level(FOTOC1) == 0)  &&  ((gpio_get_level(EM1) == 1) || (gpio_get_level(EM2) == 1))) {
            ESP_LOGI(EMERGENZE_TAG, "1");
        } else if ((n_emergenza == 2) && (gpio_get_level(EM1) == 0)     &&  ((gpio_get_level(FOTOC1) == 1) || (gpio_get_level(EM2) == 1))) {
            ESP_LOGI(EMERGENZE_TAG, "2");
        } else if ((n_emergenza == 3) && (gpio_get_level(EM2) == 0)     &&  ((gpio_get_level(EM1) == 1) || (gpio_get_level(FOTOC1) == 1))) {
            ESP_LOGI(EMERGENZE_TAG, "3");
        }*/

        EM_emergency(1);
        twai_clear_transmit_queue();
        TWAI_sendEmergenze(gpio_get_level(FOTOC1), gpio_get_level(FOTOC2), gpio_get_level(EM2), gpio_get_level(EM1), 0x01);
        ESP_LOGI(EMERGENZE_TAG, "invio emergenza diversa dall'ultima");

        if (gpio_get_level(FOTOC1) == 1) n_emergenza = 1;
        else if (gpio_get_level(EM1) == 1) n_emergenza = 2;
        else if (gpio_get_level(EM2) == 1) n_emergenza = 3;
    }


    //  PER EMERGENZE ALL'AVVIO, SE CI SONO EMERGENZE SCATTATE PRIMA DEL PRIMO RESET
    if ( (n_emergenza == 0) && (primo_reset == 0) && (Fotocellula_works == 1) ) {
        if ((gpio_get_level(FOTOC1) == 1) || (gpio_get_level(EM1) == 1) || (gpio_get_level(EM2) == 1)) {
            EM_emergency(1);
            twai_clear_transmit_queue();
            TWAI_sendEmergenze(gpio_get_level(FOTOC1), gpio_get_level(FOTOC2), gpio_get_level(EM2), gpio_get_level(EM1), 0x01);
            ESP_LOGI(EMERGENZE_TAG, "invio emergenza diversa dall'ultima");

            if (gpio_get_level(FOTOC1) == 1) n_emergenza = 1;
            else if (gpio_get_level(EM1) == 1) n_emergenza = 2;
            else if (gpio_get_level(EM2) == 1) n_emergenza = 3;
        }
    }


    //  SE FOTOCELLULA INTERROTTA DURANTE L'ABBASSA
    if (fotoc_blocc_abbassa == 1) {
        //  BLOCCA ABBASSA
        gpio_set_level(ABBASSA, 0);
        ESP_LOGE(EMERGENZE_TAG, "Abbassa OFF EMERGENZA!!");
        fotoc_blocc_abbassa = 0;
        abbassa = 0;

        //  ALZA
        gpio_set_level(ALZA, 1);    //  OUT 1
        ESP_LOGI(EMERGENZE_TAG, "Alza ON");
        t_alza = esp_timer_get_time()/1000;    //  0;
        alza = 1;

        //  INVIA EMERGENZA
        EM_emergency(0);
        twai_clear_transmit_queue();
        TWAI_sendEmergenze(gpio_get_level(FOTOC1), gpio_get_level(FOTOC2), gpio_get_level(EM2), gpio_get_level(EM1), 0x01/*!gpio_get_level(EMERGENZE)*/);
        if (gpio_get_level(FOTOC1) == 1) {
            n_emergenza = 1;
        } else if (gpio_get_level(EM1) == 1) {
            n_emergenza = 2;
        } else if (gpio_get_level(EM2) == 1) {
            n_emergenza = 3;
        }
    
        //  INVIA START AZIONE EMERGENZA
        EM_start_azione(1);
        EM_azione_1 = 1;
        t_start_az_1_int = millis();
    }


    //  CONTROLLO SU EMERGENZE DOPO AVER PREMUTO IL RESET E RISPOSTA CON EMERGENZE RESETTATE OPPURE NO
    if ((millis() - t_reset_rilasciato) < 10) {
        //if (gpio_get_level(EMERGENZE) == 1) {

        if ( (gpio_get_level(FOTOC1) == 0) && (gpio_get_level(EM2) == 0) && (gpio_get_level(EM1) == 0) ) {
            if ((emergenze == 1) || (primo_reset == 0)) {

                primo_reset = 1;

                ESP_LOGI(EMERGENZE_TAG, "RESETTATE");

                TWAI_sendEmergenze(gpio_get_level(FOTOC1), gpio_get_level(FOTOC2), gpio_get_level(EM2), gpio_get_level(EM1), 0x00);

                n_emergenza = 0;
                //ESP_LOGI(EMERGENZE_TAG, "Ho inviato 00 quindi emergenze off");

                emergenze = 0;

                if ((PID_getState(&PID) == 0) && (last_stato_pid == 1)) {
                    //stato_pid = 1;
                    pid_restart = 1;
                }

            }
            //}
 
        } /*else {
            ESP_LOGE(EMERGENZE_TAG, "NON RESETTATE");    //  ESP_LOGE(EMERGENZE_TAG, "Non ho inviato 00 quindi emergenze off");

            if (stato_motore != 2) {
                EM_emergency(1);
                twai_clear_transmit_queue();
                TWAI_sendEmergenze(gpio_get_level(FOTOC1), gpio_get_level(FOTOC2), gpio_get_level(EM2), gpio_get_level(EM1), 0x01);

                if (gpio_get_level(FOTOC1) == 1) {
                    n_emergenza = 1;
                } else if (gpio_get_level(EM1) == 1) {
                    n_emergenza = 2;
                } else if (gpio_get_level(EM2) == 1) {
                    n_emergenza = 3;
                }
            }
        }*/
    }

    //printf("EMERGENZE -------------> %d\n", gpio_get_level(EMERGENZE));       //  0 scattate e 1 emergenze resettate
    /*if ((t_pin_em != 0) && ((millis() - t_pin_em) > 200)) { 
        if (gpio_get_level(EMERGENZE) == 1) {
            TWAI_sendEmergenze(gpio_get_level(FOTOC1), gpio_get_level(FOTOC2), gpio_get_level(EM2), gpio_get_level(EM1), 0x00);
            ESP_LOGI(EMERGENZE_TAG, "Ho inviato 00 quindi emergenze off");
            t_pin_em = 0;
        }
    }*/
    

    if (!az_reset && (reset_premuto && ((millis() - t_reset_premuto) > 100))) az_reset = 1;

    //  FOTOCELLULA ____________________________________________________________________________________________________________________________________________________

    //  motore off e fotoc. interrotta --> BLINK LED ROSSO
    //  motore on e fotoc. interrotta --> BUZZER

    //  motore on e reset premuto --> BUZZER
    if ((EM_azione_1 != 0) && az_reset && (stato_motore == 1) && !fotocellula_interrotta) { ESP_LOGI(EMERGENZE_TAG, "on 1.1"); ESP_LOGI(EMERGENZE_TAG, "stato motore: %d", stato_motore); EM_start_azione(1); EM_azione_1 = 0; }
    if ((EM_azione_1 == 0) && (EM_azione == 1) && (!az_reset && (stato_motore == 1))) { ESP_LOGI(EMERGENZE_TAG, "off 1.1"); TWAI_sendAzioneEmergenze(0x00, 0x01); EM_azione = -1; EM_azione_1 = -1; }
    
    //  reset premuto e fotoc. interrotta --> BUZZER X2
    if ((EM_azione_2 != 0) && az_reset && fotocellula_interrotta) {
        ESP_LOGI(EMERGENZE_TAG, "on 2.1");
        EM_start_azione(2);
        EM_azione_2 = 0;
    }
    if ((EM_azione_2 == 0) && (EM_azione == 2) && (!az_reset || !fotocellula_interrotta)) { ESP_LOGI(EMERGENZE_TAG, "off 2.1"); TWAI_sendAzioneEmergenze(0x00, 0x02); EM_azione = -1; EM_azione_2 = -1;
                                                                                            if (emergenze == 1) { ESP_LOGI(EMERGENZE_TAG, "on 0.nuovo"); EM_start_azione(0); } }

    //  FUNGHI __________________________________________________________________________________________________________________________________________________________

    //  fungo premuto --> BUZZER
    if ((EM_azione_1 != 2) && !az_reset && fungo_premuto) { ESP_LOGI(EMERGENZE_TAG, "on 1.3"); EM_start_azione(1); EM_azione_1 = 2; }
    if ((EM_azione_1 == 2) && (EM_azione == 1) && (!fungo_premuto)) { ESP_LOGI(EMERGENZE_TAG, "off 1.3"); TWAI_sendAzioneEmergenze(0x00, 0x01); EM_azione = -1; EM_azione_1 = -1; }

    //  reset premuto e fungo premuto --> BUZZER X2
    if ((EM_azione_2 != 1) && az_reset && fungo_premuto) {
        ESP_LOGI(EMERGENZE_TAG, "on 2.2");
        EM_start_azione(2);
        EM_azione_2 = 1;
    }
    if ((EM_azione_2 == 1) && (EM_azione == 2) && (!az_reset || !fungo_premuto)) { ESP_LOGI(EMERGENZE_TAG, "off 2.2"); TWAI_sendAzioneEmergenze(0x00, 0x02); EM_azione = -1; EM_azione_2 = -1; }


    //  CONTROLLO PER QUANDO VENGONO RILASCIATI CONTEMPORANEAMENTE RESET E FOTOCELLULA
    /*if ( (t_fotocellula_rilasciata != 0) && (t_reset_rilasciato != 0) && (abs(t_fotocellula_rilasciata - t_reset_rilasciato) < 200) && (gpio_get_level(EMERGENZE) == 0)) {
        EM_emergency(0);
        twai_clear_transmit_queue();
        TWAI_sendEmergenze(0x01, gpio_get_level(FOTOC2), gpio_get_level(EM2), gpio_get_level(EM1), 0x01);
        t_rilasciati = millis();
        t_fotocellula_rilasciata = 0;
        t_reset_rilasciato = 0;

        n_emergenza = 1;

        ESP_LOGE("RILASCIATI RESET E FOTCELLULA ENTRO 100ms", "emergenza fotocellula reinviata");
    }*/

    //if ((millis() - t_rilasciati) > 500) {}


    if (pid_off_em) {
        ESP_LOGI("taskPid", "pid --> OFF");
        //PID_scrivi_file_SD("PID OFF");
        SD_operation(&SD, 1, 1, "PID OFF");
        pid_off_em = 0;
    }

    if (em_funghi) {
        SD_operation(&SD, 1, 1, "EMERGENZA FUNGHI");
        em_funghi = 0;
    }
}
