#include "PID.h"

esp_err_t PID_i2c_master_init()
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = 0;  // Set the desired I2C clock flag here

    i2c_param_config(I2C_MASTER_NUM, &conf);
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

float PID_read_temperature()
{
    uint8_t data[3];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x07, true);  // Register address of ambient temperature         AMBIENT 0x06    OBJECT 0x07
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data[0], I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &data[1], I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &data[2], I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    int16_t ambient_temp_raw = (data[1] << 8) | data[0];
    float ambient_temp_celsius = ambient_temp_raw * 0.02f - 273.15f;
    return ambient_temp_celsius;
}

void PID_temperatura_attuale_loop(Pid* pid) {
    PID_check_temperatura_attuale(&PID, temperatura_attuale);       //  INVIO TEMPERATURA (TWAI)
    PID_temp_in_SD();                                               //  SALVA TEMPERATURA (SD)
}

void PID_check_temperatura_attuale(Pid* pid, uint8_t temp_attuale) {
    /*if (last_temperatura_attuale == 0) {            //  prima temperatura da inviare

        if (temp_attuale < 200) {
            TWAI_sendStatoPid(PID_getState(&PID), temp_attuale);
            last_temperatura_attuale = temp_attuale;
        } else {
            ESP_LOGE(PID_TAG, "(last_temperatura_attuale = 0) e (temperatura_attuale >= 200) --> temperatura attuale = %d", temp_attuale);
            cont_prima_temp_errata++;
        }

        if (cont_prima_temp_errata == 6) {
            ESP_LOGE(PID_TAG, "Ho letto le prime 6 temperature ed erano tutte superiori a 200° --> ERRORE SENSORE DI TEMPERATURA");
            PID_stop(&PID);

            TWAI_sendStatoPid(PID_getState(&PID), temp_attuale);
            TWAI_sendError(0x01);
            
            SD_operation(&SD, 1, 1, "ERRORE SENSORE DI TEMPERATURA\nPID OFF");

            cont_prima_temp_errata = 0;
        }

    } else if (last_temperatura_attuale > 0) {      //  ho già inviato la prima temperatura

        if (abs(temp_attuale - last_temperatura_attuale) <= 20) {

            //  if(abs(temp_attuale - last_temperatura_attuale) <= 1) {}

            TWAI_sendStatoPid(PID_getState(&PID), temp_attuale);
            last_temperatura_attuale = temp_attuale;
            cont_temp_errata = 0;

        } else {
            
            ESP_LOGE(PID_TAG, "(last_temperatura_attuale != 0) e (temperatura_attuale - last_temperatura_attuale >= 20) --> temperatura attuale = %d", temp_attuale);

            //  SCRIVI IN SD TEMPERATURA ERRATA
            char char_errore[5] = ">20°";
            char char_temp[5];   snprintf(char_temp, sizeof(char_temp), "%d", temp_attuale);
            char string[15];
            strcpy(string, char_errore);
            strcat(string, ";");
            strcat(string, char_temp);
            //PID_scrivi_file_SD(string);
            SD_operation(&SD, 1, 1, string);

            cont_temp_errata++;
            //  SCRIVI IN SD ERRORE SE HO RICEVUTO 6 TEMPERATURE ERRATE DI SEGUITO
            if (cont_temp_errata == 6) {
                //if ((millis() - t_pidoff) > 100) {
                ESP_LOGE(PID_TAG, "Ho letto 6 temperature che avevano differenza > 10 rispetto alla precedente --> ERRORE SENSORE DI TEMPERATURA");

                PID_stop(&PID);
                ESP_LOGI("taskPid", "pid --> OFF");

                TWAI_sendStatoPid(PID_getState(&PID), temp_attuale);
                TWAI_sendError(0x01);
                
                SD_operation(&SD, 1, 1, "ERRORE SENSORE DI TEMPERATURA\nPID OFF");

                t_pidoff = millis();//}

                cont_temp_errata = 0;
            }
        }

    }*/

    //TWAI_sendStatoPid(PID_getState(&PID), temp_attuale);
    //last_temperatura_attuale = temp_attuale;

    //  "STABILIZZATORE" SULLA VISUALIZZAZIONE DEL VALORE DELLA TEMPERATURA
    cont_display_0++;

    if (cont_display_0 <= 10) {                                                                 //  le prime 10 letture che faccio le mando tutte per visualizzarle

        TWAI_sendStatoPid(PID_getState(&PID), temperatura_attuale);
        last_send_temperature = millis();
        display_temperature = temperatura_attuale;
        
    } else {

        if ((PID_getState(&PID) == 1) && (target_temperature - temperatura_attuale == 1)) {     //  se leggo 10 volte la temperatura con 1° di differenza sotto il target --> invio target
            
            cont_target_diff_1++;
            if (cont_target_diff_1 == 20) {
                TWAI_sendStatoPid(PID_getState(&PID), temperatura_attuale);
                last_send_temperature = millis();
                display_temperature = temperatura_attuale;
                cont_diff_1 = 0;
                cont_diff_2 = 0;
            }
            if (cont_target_diff_1 == 40) {
                cont_target_diff_1 = 21;    //  0;
                if (target_temperature != display_temperature) {
                    TWAI_sendStatoPid(PID_getState(&PID), target_temperature);
                    last_send_temperature = millis();
                    display_temperature = temperatura_attuale;
                    cont_diff_1 = 0;
                    cont_diff_2 = 0;
                }
            }

        } else {

            if (abs(temperatura_attuale - display_temperature) == 1) {                      //  invio se leggo 10 volte la temperatura con differenza == 1 dalla temperatura visualizzata a display
                cont_diff_1++;
                if (cont_diff_1 == 10) {
                    TWAI_sendStatoPid(PID_getState(&PID), temperatura_attuale);
                    last_send_temperature = millis();
                    display_temperature = temperatura_attuale;
                    cont_diff_1 = 0;
                    cont_diff_2 = 0;
                    cont_target_diff_1 = 0;
                    //if (cont_target_diff_1 >= 21) cont_target_diff_1 = 0;
                }
            } else if (abs(temperatura_attuale - display_temperature) > 1) {                       //  invio se leggo 10 volte la temperatura con differenza > 1 dalla temperatura visualizzata a display
                cont_diff_2++;
                if (cont_diff_2 == 10) {
                    TWAI_sendStatoPid(PID_getState(&PID), temperatura_attuale);
                    last_send_temperature = millis();
                    display_temperature = temperatura_attuale;
                    cont_diff_1 = 0;
                    cont_diff_2 = 0;
                    cont_target_diff_1 = 0;
                    //if (cont_target_diff_1 >= 11) cont_target_diff_1 = 0;
                }
            }

        }

        if ((millis() - last_send_temperature) > 30000) {                                       //  se non invio la temperatura da più di 30 secondi
            TWAI_sendStatoPid(PID_getState(&PID), temperatura_attuale);
            last_send_temperature = millis();
            display_temperature = temperatura_attuale;
        }
    }
}

/*void PID_send_temperatura_iniziale(Pid* pid) {
    temperatura_attuale = PID_read_temperature();

    if (segno_offset) temperatura_attuale = temperatura_attuale - pid->OFFSET;
    else temperatura_attuale = temperatura_attuale + pid->OFFSET;

    temperatura_attuale = (int)ceil(temperatura_attuale);

    if (temperatura_attuale < 200) { TWAI_sendStatoPid(PID_getState(&PID), temperatura_attuale); } 
    else { ESP_LOGE(PID_TAG, "(temperatura_attuale >= 200) --> temperatura attuale = %d", temperatura_attuale); }
}*/

void PID_control_heater(Pid* pid, float c_temp)
{
    pid->curr_temp = c_temp;
    pid->PID_value = PID_calculateValue(&PID);
    if ((millis() - pid->SSR_time) >= pid->SSR_period) {
        pid->SSR_time = millis();
    }
    if (pid->state && ((millis() - pid->SSR_time) > (pid->SSR_period * (255 - pid->PID_value) / 255))) {
        gpio_set_level(pid->SSR_PIN, 1);
    } else {
        gpio_set_level(pid->SSR_PIN, 0);
    }

    /*if (watchdog)*/ esp_task_wdt_reset_user(func_twdt_user_hdl);  //  ESP_LOGI("TWDT", "reset");
}

void PID_task(Pid* pid)
{
    temperature = PID_read_temperature();       //  temperatura "raw"
    //  printf("Temperature: %.2f°C\n", temperature);


    //  CONTROLLO SU LETTURA SENSORE DI TEMPERATURA
    if (display_temperature == 0) {            //  prima temperatura da inviare

        if ((temperature < -10.0) || (temperature > 200.0)) {
            ESP_LOGE(PID_TAG, "(temperature < -10.0) || (temperature > 200.0) --> temperatura attuale = %lf", temperature);
            cont_prima_temp_errata++;
            accept_temperature = 0;
        } else {
            cont_prima_temp_errata = 0;
            accept_temperature = 1;
        }

    } else if (display_temperature > 0) {      //  ho già inviato la prima temperatura

        if (abs((int)roundf(temperature) - display_temperature) <= 20) {
            cont_temp_errata = 0;
            accept_temperature = 1;
        } else {
            ESP_LOGE(PID_TAG, "(display_temperature != 0) e (temperatura_attuale - display_temperature >= 20) --> temperatura attuale = %d", (int)roundf(temperature));
            cont_temp_errata++;
            accept_temperature = 0;
            //temperature = last_temperature;
            //  SCRIVI IN SD TEMPERATURA ERRATA
            char char_errore[5] = ">20°";
            char char_temp[5];   snprintf(char_temp, sizeof(char_temp), "%d", (int)roundf(temperature));
            char string[15];
            strcpy(string, char_errore);
            strcat(string, ";");
            strcat(string, char_temp);
            SD_operation(&SD, 1, 1, string);
        }

    }


    if (accept_temperature == 1) {

        last_temperature = temperature;
        
        if (segno_offset) temperature_offset = temperature - pid->OFFSET;
        else temperature_offset = temperature + pid->OFFSET;

        temperatura_attuale = (int)roundf(temperature_offset);   //  temperatura attuale (calcolata con l'offset)       //  ceil() arrontondamento per eccesso (es. 29,05 --> 30)

        PID_control_heater(&PID, temperature_offset/*temperatura_attuale*/);

    } else {

        if (cont_prima_temp_errata == 6) {
            ESP_LOGE(PID_TAG, "Ho letto le prime 6 temperature ed erano tutte errate --> ERRORE SENSORE DI TEMPERATURA");
            if (PID_getState(&PID) == 1) { PID_stop(&PID); ESP_LOGI("taskPid", "pid --> OFF"); t_pidoff = millis(); }
            TWAI_sendStatoPid(PID_getState(&PID), 0);
            TWAI_sendError(0x01);
            SD_operation(&SD, 1, 1, "ERRORE SENSORE DI TEMPERATURA\nPID OFF");
            cont_prima_temp_errata = 0;
        }

        if (cont_temp_errata == 6) {
            ESP_LOGE(PID_TAG, "Ho letto 6 temperature che avevano differenza > 20 rispetto alla precedente --> ERRORE SENSORE DI TEMPERATURA");
            if (PID_getState(&PID) == 1) { PID_stop(&PID); ESP_LOGI("taskPid", "pid --> OFF"); t_pidoff = millis(); }
            TWAI_sendStatoPid(PID_getState(&PID), 0);
            TWAI_sendError(0x01);
            SD_operation(&SD, 1, 1, "ERRORE SENSORE DI TEMPERATURA\nPID OFF");
            cont_temp_errata = 0;
        }

    }

    //  ---

    /*temperature = PID_read_temperature();                       //  temperatura "raw"
    
    if (pid->OFFSET > 0) {
        if (segno_offset) {
            temperature_offset = temperature - pid->OFFSET;
        } else {
            temperature_offset = temperature + pid->OFFSET;
        }
        temperatura_attuale = (int)ceil(temperature_offset);    //  temperatura attuale (calcolata con l'offset)
    } else {
        temperatura_attuale = (int)ceil(temperature);           //  temperatura attuale (senza offset)
    }
    
    PID_control_heater(&PID, temperatura_attuale);*/
}

void PID_begin(Pid* pid, uint8_t SSR_pin, float kp_, float ki_, float kd_) {
    pid->SSR_PIN = SSR_pin;
    PID_setOutPeriod(&PID, 2000);
    gpio_pad_select_gpio(pid->SSR_PIN);
    gpio_set_direction(pid->SSR_PIN, GPIO_MODE_OUTPUT);
    PID_setPidParameters(&PID, kp_, ki_, kd_);
}

void PID_setPidParameters(Pid* pid, float kp_, float ki_, float kd_) {
    pid->KP = kp_;
    pid->KI = ki_;
    pid->KD = kd_;
}

void PID_setOffset(Pid* pid, float offset_) {
    pid->OFFSET = offset_;
}

void PID_setOutPeriod(Pid* pid, uint32_t period) {
    pid->SSR_period = period;
}

void PID_setTemp(Pid* pid, uint8_t t_set) {
    if (pid->state) {
        PID_start(pid, t_set);
    } else {
        pid->set_temp = t_set;
    }
}

void PID_start(Pid* pid, uint8_t t_set) {
    if (!pid->state || pid->set_temp != t_set) {
        pid->set_temp = t_set;
        pid->PID_p = 0;
        pid->PID_i = 0;
        pid->PID_d = 0;
        pid->start_time = millis() / 1000.0;
        pid->state = 1;
    }
}

void PID_stop(Pid* pid) {
    //  set_temp = 0; non resettare la temperatura impostata
    gpio_set_level(pid->SSR_PIN, 0);
    pid->PID_p = 0;
    pid->PID_i = 0;
    pid->PID_d = 0;
    pid->state = 0;

    prima_rampa_fatta = 0;
}


uint8_t PID_getSetTemp(Pid* pid) {
    return pid->set_temp;
}

uint8_t PID_getState(Pid* pid) {
    uint8_t stato;
    if (pid->state) stato = 1; else stato = 0;
    return stato;
}

float PID_getKp(Pid* pid) {
    return pid->KP;
}

float PID_getKi(Pid* pid) {
    return pid->KI;
}

float PID_getKd(Pid* pid) {
    return pid->KD;
}

float PID_getOffset(Pid* pid) {
    return pid->OFFSET;
}

float PID_calculateValue(Pid* pid) {
    if (prima_rampa_fatta == 1) { pid->curr_temp = pid->curr_temp - 2; /*ESP_LOGI(PID_TAG, "-2");*/ }   // -----------------------------------------------------------------------------

    float PID_v;
    float PID_error = pid->set_temp - pid->curr_temp;
    
    if ((PID_getState(&PID) == 1) && (prima_rampa_fatta == 0) && (PID_error <= 0.0)) { prima_rampa_fatta = 1; /*ESP_LOGI(PID_TAG, "prima_rampa_fatta = 1");*/ }     // -----------------
    
    pid->timePrev = pid->Time;
    pid->Time = millis();
    pid->elapsedTime = (float)(pid->Time - pid->timePrev) / 1000.0;

    //  PROPORZIONALE
    pid->PID_p = pid->KP * PID_error;

    //  INTEGRALE
    /*pid->PID_i = pid->PID_i + (pid->KI * PID_error * pid->elapsedTime);

    if (PID_error <= 0.0) {
        //  ESP_LOGW(PID_TAG, "pid->PID_i = 0;");
        pid->PID_i = 0;
    }

    //  DERIVATIVA
    pid->PID_d = pid->KD * ((float)(PID_error - pid->previous_error) / pid->elapsedTime);*/
    //  0,1  0,01  0,5 --> 1 , 1 , 5 da inserire

    if (PID_error < -5) {   //  if (PID_error < -5) { // se temperatura effettiva superiore di 5 gradi rispetto t_set
        pid->cooling = 1; // sono in raffreddamento quindi disabilito integrale
        
        //pid->PID_i = 0;   //  modifica sbagliata
    } else if (PID_error > 0) { //altrimenti se scendo sotto t_set
        pid->cooling = 0; // non sono più in raffreddamento
    }

    //  INTEGRALE
    if ((PID_error < 10) && (pid->KI != 0) && (!pid->cooling))
    {
        pid->PID_i = pid->PID_i + (pid->KI * (float)PID_error / 200.0);
        //pid->PID_i = pid->PID_i + (pid->KI * PID_error * pid->elapsedTime);   //  nuova formula integrale
    } else {
        pid->PID_i = 0;
    }

    //  DERIVATIVA
    if (PID_error < 20)
    {
        pid->PID_d = pid->KD * ((float)(PID_error - pid->previous_error) / pid->elapsedTime);
    } else {
        pid->PID_d = 0;
    }

    pid->previous_error = PID_error;

    //  OUTPUT
    PID_v = (float)pid->PID_p + (float)pid->PID_i + (float)pid->PID_d;

    if (PID_v < 0)
    {
        PID_v = 0;
    }
    if (PID_v > 255)
    {
        PID_v = 255;
    }

    if (!pid->state) {
        PID_v = 0;
        pid->PID_p = 0;
        pid->PID_i = 0;
        pid->PID_d = 0;
    }

    /*if (pid->state) {
        printf("kp: %f ,", pid->KP);
        printf(" ki: %f ,", pid->KI);
        printf(" kd: %f  -  ", pid->KD);

        printf("stato: %d", pid->state);
        printf(" target: %f", pid->set_temp);
        printf(" curr: %f  - ", pid->curr_temp);

        printf(" PID: P: %f", pid->PID_p);
        printf(" I: %f", pid->PID_i);
        printf(" D: %f", pid->PID_d);
        printf(" Val: %f\n", PID_v);
    }*/

    return PID_v;
}

//  minuti dall'avvio ; stato pid(on/off) ; temp. target ; temp. media nel minuto
void PID_temp_in_SD() {
    t_minuti = millis() / 60000;

    somma_temperatura_attuale = somma_temperatura_attuale + temperatura_attuale;
    cont_somma_temperatura_attuale ++;

    if (((millis() - t_temperatura_sd) > 60000) && ((millis() - t_valori_cambiati) > 1000)) {        //  SALVA TEMPERATURA (SD)
        char char_minuti[10];   snprintf(char_minuti, sizeof(char_minuti), "%ld", t_minuti);

        char char_stato[10];    snprintf(char_stato, sizeof(char_stato), "%d", PID_getState(&PID));

        char char_target[11];   snprintf(char_target, sizeof(char_target), "%d°", target_temperature);

        char char_media[11];
        media_temperatura_attuale = (int)roundf(somma_temperatura_attuale/cont_somma_temperatura_attuale);
        snprintf(char_media, sizeof(char_media), "%d°", media_temperatura_attuale);

        char string[50];
        strcpy(string, char_minuti);
        strcat(string, ";");
        strcat(string, char_stato);
        strcat(string, ";");
        strcat(string, char_target);
        strcat(string, ";");
        strcat(string, char_media);

        //PID_scrivi_file_SD(string);
        SD_operation(&SD, 1, 1, string);

        media_temperatura_attuale = 0;
        somma_temperatura_attuale = 0;
        cont_somma_temperatura_attuale = 0;

        t_temperatura_sd = esp_timer_get_time()/1000;
    }
    
}

void PID_avvio_in_SD() {
    //PID_scrivi_file_SD("\nAVVIO MACCHINA");
    SD_operation(&SD, 1, 1, "\nAVVIO MACCHINA");
}

/*void PID_scrivi_file_SD(char *s) {
    if(!SD.busy) {
        SD_write(&SD, 1, s);
    } else {
        ESP_LOGE(TAG_sd, "SD in uso, non sono riuscito a salvare i valori temperatura nel file");
        bool scritto = false;
        while(!scritto) {
            if(!SD.busy) {
                SD_write(&SD, 1, s); 
                scritto = true;
            }
            delay(10);
        }
    }
}*/


//  WATCHDOG

void PID_init_WDT() {
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = WDT_TIMEOUT,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,    // Bitmask of all cores
        .trigger_panic = false,
    };
    ESP_ERROR_CHECK(esp_task_wdt_init(&twdt_config));
    ESP_LOGI("TWDT", "TWDT initialized");

    // Subscribe this task to TWDT, then check if it is subscribed
    //ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    //ESP_ERROR_CHECK(esp_task_wdt_status(NULL));
    // Subscribe func as users of the the TWDT
    ESP_ERROR_CHECK(esp_task_wdt_add_user("func_PID_check_WDT", &func_twdt_user_hdl));
    ESP_LOGI("TWDT", "func subscribed to TWDT");

    watchdog = 1;
}

void PID_deinit_WDT() {
    watchdog = 0;

    ESP_ERROR_CHECK(esp_task_wdt_delete_user(func_twdt_user_hdl));
    //ESP_ERROR_CHECK(esp_task_wdt_delete(NULL));
    ESP_LOGI("TWDT", "func unsubscribed");

    ESP_ERROR_CHECK(esp_task_wdt_deinit());
    ESP_LOGI("TWDT", "TWDT deinitialized");
}

/*void PID_check_WDT(Pid* pid) {
    if (watchdog) {
        if (gpio_get_level(pid->SSR_PIN) == 0) {
            //esp_task_wdt_reset();
            esp_task_wdt_reset_user(func_twdt_user_hdl);
        } else TWAI_sendError(0x01);
    }
}*/