#include "PEDALE.h"

void PEDALE_init() {
    gpio_set_direction(PEDALE_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(RELE_PIN, GPIO_MODE_INPUT_OUTPUT);
}

void PEDALE_loop() {

    //  PEDALE  O  TARTARUGA
    /*if (stato_pedale == 1 || stato_tartaruga == 1) {
        if (ped_pre_al_pass_mod_ped != 1) {
            if (pedale == 1) {
                pedale_premuto = 1;

                //ESP_LOGE("!", "stato_pedale: %d, stato_tartaruga: %d, ped_pre_al_pass_mod_ped: %d, pedale: %d, pedale_premuto: %d, ultimo_pedale_premuto: %d", stato_pedale, stato_tartaruga, ped_pre_al_pass_mod_ped, pedale, pedale_premuto, ultimo_pedale_premuto);

                if (pedale_premuto != ultimo_pedale_premuto) {
                    ESP_LOGI("PEDALE", "premuto");
                    if (stato_pedale == 1) TWAI_sendStatoPedale(0x01, 0x01);  //  premuto
                    else if (stato_tartaruga == 1) TWAI_sendStatoPedale(0x00, 0x01);

                    if (pedale_premuto && allow_pedale) {
                        
                        if (gpio_get_level(GPIO_NUM_38) == 1) {
                            allow_photocell = 1;
                            if (stato_pedale) {
                                ESP_LOGI("PEDALE", "PEDALE -> velocià impostata");
                                TWAI_sendMotorCommand(MOTOR_ON);
                            } else if (stato_tartaruga) {
                                ESP_LOGI("PEDALE", "TARTARUGA -> velocità tartaruga");
                                TWAI_sendMotorCommand(MOTOR_TURTLE);

                                //  disattiva fotocellule, disattivando l interrupt e simulando il tasto reset tenuto premuto
                                gpio_intr_disable(GPIO_NUM_48);
                                gpio_set_direction(GPIO_NUM_48, GPIO_MODE_OUTPUT);
                                gpio_set_level(GPIO_NUM_48, 0);
                            }   
                        } else {
                            ESP_LOGE("PEDALE", "ACCENDI IL MOTORE non riuscito --> emergenze attivate");
                        }
                    }
                }
                ultimo_pedale_premuto = pedale_premuto;

            } else {
                pedale_premuto = 0;

                if (pedale_premuto != ultimo_pedale_premuto) {
                    ESP_LOGI("PEDALE", "rilasciato");
                    if (stato_pedale == 1) TWAI_sendStatoPedale(0x01, 0x00);  //  rilasciato
                    else if (stato_tartaruga == 1) TWAI_sendStatoPedale(0x00, 0x00);

                    ESP_LOGI("PEDALE", "SPEGNI IL MOTORE");
                    //TWAI_sendMotorCommand(0x00);
                    TWAI_sendMotorCommand(MOTOR_OFF);

                    if (stato_tartaruga) {
                        //  riattiva fotocellule, lasciando il tasto reset non premuto e riattivando l interrupt
                        gpio_set_level(GPIO_NUM_48, 1);     //  reset
                        delay(100);
                        gpio_set_direction(GPIO_NUM_48, GPIO_MODE_INPUT);
                        gpio_intr_enable(GPIO_NUM_48);
                    }
                    allow_photocell = 0;
                }
                ultimo_pedale_premuto = pedale_premuto;
            }
        } else {
            if (pedale == 0) {
                ped_pre_al_pass_mod_ped = 0;
            }
        }
    }*/


    //  PEDALE
    if (stato_pedale) {
        if (pedale) {
            if (funzione_premuto) {
                ESP_LOGI("PEDALE", "premuto");
                TWAI_sendStatoPedale(0x01, 0x01);  //  premuto
                if (allow_pedale) {
                    allow_photocell = 1;
                    ESP_LOGI("PEDALE", "PEDALE -> velocià impostata");
                    TWAI_sendMotorCommand(MOTOR_ON);
                } else ESP_LOGE("PEDALE", "stato_pedale --> allow_pedale == 0");
                funzione_premuto = 0;
                funzione_rilasciato = 1;
                //if (!da_man_a_ped) funzione_rilasciato = 1;
            }
        } else {
            if (funzione_rilasciato) {
                ESP_LOGI("PEDALE", "rilasciato");
                TWAI_sendStatoPedale(0x01, 0x00);  //  rilasciato
                ESP_LOGI("PEDALE", "SPEGNI IL MOTORE");
                TWAI_sendMotorCommand(MOTOR_OFF);
                allow_photocell = 0;
                funzione_premuto = 1;
                funzione_rilasciato = 0;
            }
        }
    }


    //  TARTARUGA
    if (stato_tartaruga) {
        if (pedale) {
            if (funzione_premuto) {
                ESP_LOGI("PEDALE", "premuto");
                TWAI_sendStatoPedale(0x00, 0x01);    //  premuto
                if (allow_pedale) {
                    //allow_photocell = 1;
                    ESP_LOGI("PEDALE", "TARTARUGA -> velocità tartaruga");
                    TWAI_sendMotorCommand(MOTOR_TURTLE);

                    //  disattiva fotocellule, disattivando l interrupt e simulando il tasto reset tenuto premuto
                    gpio_intr_disable(GPIO_NUM_48);
                    gpio_set_direction(GPIO_NUM_48, GPIO_MODE_OUTPUT);
                    gpio_set_level(GPIO_NUM_48, 0);
                    fotoc_dis_da_reset = 1;
                } else ESP_LOGE("PEDALE", "stato_tartaruga --> allow_pedale == 0");
                funzione_premuto = 0;
                funzione_rilasciato = 1;
            }
        } else {
            if (funzione_rilasciato) {
                ESP_LOGI("PEDALE", "rilasciato");
                TWAI_sendStatoPedale(0x00, 0x00);    //  rilasciato
                ESP_LOGI("PEDALE", "SPEGNI IL MOTORE");
                TWAI_sendMotorCommand(MOTOR_OFF);
                allow_photocell = 0;
                funzione_premuto = 1;
                funzione_rilasciato = 0;

                //  riattiva fotocellule, lasciando il tasto reset non premuto e riattivando l interrupt
                gpio_set_level(GPIO_NUM_48, 1);     //  reset
                delay(100);
                gpio_set_direction(GPIO_NUM_48, GPIO_MODE_INPUT);
                gpio_intr_enable(GPIO_NUM_48);
                fotoc_dis_da_reset = 0;
            }
        }
    }


    //  S1
    if (stato_s1 == 1) {
        if (!funzione_s1) {                                             //  SMART PEDAL
            if (pedale) {     //  if (gpio_get_level(PEDALE_PIN) == 0) {
                if (funzione_premuto) {
                    ESP_LOGI("SMART PEDAL", "premuto");
                    TWAI_sendStatoPedale(0x00, 0x01);  //  premuto

                    if (t_alza >= t_abbassa) {
                        t_msg_abbassa = millis();
                        if (allow_abbassa == 1) {
                            if (gpio_get_level(FOTOC1) == 0) {
                                gpio_set_level(ABBASSA, 1); //  OUT 2
                                ESP_LOGI("SMART PEDAL", "Abbassa ON");
                                t_abbassa = esp_timer_get_time()/1000;    //  0;
                                abbassa = 1;
                            } else {
                                ESP_LOGI("SMART PEDAL", "invio emergenze per fotocellula occupata quando ho ricevuto ABBASSA");
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
                                EM_start_azione(1);
                                EM_azione_1 = 1;
                                t_start_az_1_int = millis();
                            }
                        } else {
                            ESP_LOGE("SMART PEDAL", "ABBASSA non riuscito --> emergenze attivate");
                        }
                    }


                    funzione_premuto = 0;
                    funzione_rilasciato = 1;
                }

            } else {
                if (funzione_rilasciato) {
                    ESP_LOGI("SMART PEDAL", "rilasciato");
                    TWAI_sendStatoPedale(0x00, 0x00);  //  al rilascio
                    stato_s1 = 0;
                    funzione_premuto = 1;
                    funzione_rilasciato = 0;
                }
            }

        } else {                                                        //  RELE'
            if (gpio_get_level(RELE_PIN) == 0) {
                gpio_set_level(RELE_PIN, 1);
                ESP_LOGI("RELE' S1", "attivato");
            }
        }

    } else {
        if (funzione_s1) {
            if (gpio_get_level(RELE_PIN) == 1) {
                gpio_set_level(RELE_PIN, 0);
                ESP_LOGE("RELE' S1", "disattivato");
            }
        }

    }


}
