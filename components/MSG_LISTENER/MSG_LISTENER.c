#include "MSG_LISTENER.h"

void MSG_lettura_in_SPIFF() {
    if (!lettura_ore_in_SPIFF) {
        if ((millis() - t_sd_closed) > 100) {
            if (!crea_file_in_SPIFF) {
                int8_t res = SPIFFS_read_(&SPIFFS); 
                switch (res) { 
                    case 0:
                        lettura_ore_in_SPIFF = 1; 
                        ESP_LOGI(MSG_TAG, "millis contatori letti da SPIFF");
                        break;

                    case 1:
                        crea_file_in_SPIFF = 1;
                        t_sd_closed = millis();
                        break;

                    default:
                        break;
                }
            } else {
                if (SPIFFS_write_(&SPIFFS) == ESP_OK) {
                    ESP_LOGI(MSG_TAG, "creato il millis.csv (millis contatori) in SPIFF");
                    lettura_ore_in_SPIFF = 1;
                }
                
            }
            t_sd_closed = millis();
        }
    }
}


void macchina_ready() {
    t_restart_avvio = millis();

    while(!macchina_pronta) {

        if ((millis() - t_restart_avvio) > 9000) {     //  timeout
            if (pingOK) { ESP_LOGE("macchina_ready", "ping ricevuto ma valori non gestiti"); } else { ESP_LOGE("macchina_ready", "ping non ricevuto"); } ESP_LOGI("macchina_ready", "RESTART ESP...");
            delay(1000);
            esp_restart();
        }

        if(TWAI_get() == ESP_OK) {
            if(message.identifier != 0) {
                priorita = (message.identifier >> 8) & 0x7;
                mittente = (message.identifier >> 4) & 0xF;
                destinatario = message.identifier & 0xF;

                if ((destinatario == 2) || (destinatario == 4))
                {
                    switch (priorita) {
                        case 1:
                            if (pingOK) {
                                if (!mi_sono_riavviato && SD_works) {
                                    if (((destinatario == 2) || (destinatario == 4)) && (buf[0] == 0x0D)) {                                                   //  RICEZIONE RICHIESTE VALORI
                                        TWAI_sendValoriMotore(mittente, velocita_minima, velocita_massima, velocita_reverse, velocita_tartaruga, velocita_impostata, corrente_massima);
                                        if (mittente == 1) {
                                            ESP_LOGI(MSG_TAG, "richiesta valori esp1 ricevuta");
                                            TWAI_sendValoriRiscaldamento1(temperatura_minima, temperatura_massima, target_temperature);
                                            kp_ = kp*10;   ki_ = ki*100;   kd_ = kd*10;   offset_ = offset/**10*/;   
                                            if (segno_offset) segno_offset_ = 1; else segno_offset_ = 0;
                                            ESP_LOGI("invio valori pid2 iniziali", "kp: %d - ki: %d - kd: %d - offset: %d", kp_, ki_, kd_, offset_);
                                            TWAI_sendValoriRiscaldamento2(kp_, ki_, kd_, offset_, segno_offset_);
                                            richiesta_esp1 = 1;
                                        } else if (mittente == 3) {
                                            ESP_LOGI(MSG_TAG, "richiesta valori esp3 ricevuta");
                                            richiesta_esp3 = 1;
                                        }

                                        if (richiesta_esp1 && richiesta_esp3) {     //  se ho risposto a entrambi gli esp e tutti hanno i valori sincronizzati
                                            ESP_LOGI(MSG_TAG, "richieste ricevute! MACCHINA PRONTA\n");
                                            macchina_pronta = 1;
                                            delay(10);
                                        }
                                    }
                                }
                            }
                            break;

                        case 2:
                            if (pingOK) {
                                if (mittente == 1) {
                                    if (buf[0] == 0x00) {                                 //  RICEZIONE STATO MACCHINA
                                        ESP_LOGE(MSG_TAG, "Mi sono riavviato --> ho ricevuto lo stato della macchina da esp1");
                                        mi_sono_riavviato = 1;
                                        TWAI_sendAzioneEmergenze(0x01, 0x00);

                                        //  buf[1]
                                        stato_motore = buf[2];
                                        stato_pedale = buf[3];
                                        switch(buf[4]) {
                                            case 0:
                                                emergenze_funghi = 0;
                                                emergenze_fotocellule = 0;
                                                break;
                                            case 1:
                                                emergenze_funghi = 1;
                                                emergenze_fotocellule = 0;
                                                break;
                                            case 2:
                                                emergenze_funghi = 0;
                                                emergenze_fotocellule = 1;
                                                break;
                                        }
                                        //if (buf[5] == 0) { PID_stop(&PID); ESP_LOGI("taskPid", "pid --> OFF"); SD_operation(&SD, 1, 1, "PID OFF");/*PID_scrivi_file_SD("PID OFF");*/ } else PID_start_riavvio = 1;
                                        if (buf[6] == 0) stato_s1 = 1; else stato_s1 = 0;
                                        funzione_s1 = buf[7];

                                        //TWAI_sendRichiestaValori(0x02);
                                        ESP_LOGI("stato macchina", "stato_motore --> %d", stato_motore);
                                        ESP_LOGI("stato macchina", "stato_pedale --> %d", stato_pedale);
                                        ESP_LOGI("stato macchina", "stato_pid --> %d", PID_getState(&PID));
                                        ESP_LOGI("stato macchina", "stato_s1 --> %d", stato_s1);
                                        ESP_LOGI("stato macchina", "funzione_s1 --> %d\n", funzione_s1);
                                    }

                                    if (mi_sono_riavviato || !SD_works) {
                                        if (buf[0] == 0x0D) {           //  RICEZIONE VALORI
                                            switch(buf[1]) {
                                                case 0:
                                                    velocita_minima = buf[2];
                                                    velocita_massima = buf[3];
                                                    velocita_reverse = buf[4];
                                                    velocita_tartaruga = buf[5];
                                                    velocita_impostata = buf[6];
                                                    corrente_massima = buf[7];
                                                    valori_motore = 1;
                                                    break;
                                                
                                                case 1:
                                                    temperatura_minima = buf[2];
                                                    temperatura_massima = buf[3];
                                                    target_temperature = buf[4];    if(PID_start_riavvio == 1) PID_start(&PID, target_temperature); ESP_LOGI("taskPid", "pid --> ON"); SD_operation(&SD, 1, 1, "PID ON");/*PID_scrivi_file_SD("PID ON");*/
                                                    valori_pid_1 = 1;
                                                    break;

                                                case 2:
                                                    kp = buf[2]/10.0;
                                                    ki = buf[3]/100.0;
                                                    kd = buf[4]/10.0;
                                                    offset = buf[5]/*/10.0*/;
                                                    if (buf[6] == 0x00) segno_offset = 0; else if (buf[6] == 0x01) segno_offset = 1;
                                                    valori_pid_2 = 1;
                                                    break;
                                            }

                                            if (valori_motore && valori_pid_1 && valori_pid_2) {
                                                MSG_valori_in_SD();

                                                ESP_LOGI(MSG_TAG, "valori ricevuti! MACCHINA PRONTA\n");

                                                //  VALORI MOTORE
                                                ESP_LOGI("valori ricevuti", "v_min --> %d", velocita_minima);
                                                ESP_LOGI("valori ricevuti", "v_max --> %d", velocita_massima);
                                                ESP_LOGI("valori ricevuti", "v_rev --> %d", velocita_reverse);
                                                ESP_LOGI("valori ricevuti", "v_tar --> %d", velocita_tartaruga);
                                                ESP_LOGI("valori ricevuti", "v_imp --> %d", velocita_impostata);
                                                ESP_LOGI("valori ricevuti", "c_max --> %d", corrente_massima);
                                                //  VALORI PID 1
                                                ESP_LOGI("valori ricevuti", "t_min --> %d", temperatura_minima);
                                                ESP_LOGI("valori ricevuti", "t_max --> %d", temperatura_massima);
                                                ESP_LOGI("valori ricevuti", "t_imp --> %d", target_temperature);
                                                //  VALORI PID 2
                                                ESP_LOGI("valori ricevuti", "kp    --> %f", kp);
                                                ESP_LOGI("valori ricevuti", "ki    --> %f", ki);
                                                ESP_LOGI("valori ricevuti", "kd    --> %f", kd);
                                                ESP_LOGI("valori ricevuti", "offset--> %f", offset);
                                                if (segno_offset == 1) ESP_LOGI("valori ricevuti", "segno_offset--> -\n"); else ESP_LOGI("valori ricevuti", "segno_offset--> +\n");

                                                macchina_pronta = 1;
                                                delay(10);
                                            }
                                        }
                                    }
                                }
                            }
                            break;

                        case 3:
                            switch (buf[0]) {
                                case PING_REQUEST:
                                    ESP_LOGI(PING_TAG, "PING request da esp%d", mittente);
                                    TWAI_pingReply(versioneFW, mittente);
                                    /*switch (mittente) {
                                        case 1: pingRequest_esp1 = 1; break;
                                        case 3: pingRequest_esp3 = 1; break;
                                        default: ESP_LOGE(PING_TAG, "mittente non riconosciuto"); break;
                                    }
                                    if (pingRequest_esp1 && pingRequest_esp3) {
                                        pingRequest_received = 1;
                                    }*/
                                    break;

                                case PING_REPLY:
                                    ESP_LOGI(PING_TAG, "PING reply da esp%d", mittente);
                                    switch (mittente) {
                                        case 1: pingReply_esp1 = 1; versioneFW_esp1 = buf[1]; break;
                                        case 3: pingReply_esp3 = 1; versioneFW_esp3 = buf[1]; break;
                                        default: ESP_LOGE(PING_TAG, "mittente non riconosciuto"); break;
                                    }
                                    if (pingReply_esp1 && pingReply_esp3) {
                                        pingReply_received = 1;
                                    }
                                    break;

                                default:
                                    ESP_LOGE(PING_TAG, "ERROR --> byte0 = %u", buf[0]);
                                    break;
                            }
                            if (!pingOK && /*pingRequest_received && */pingReply_received) {     //  se all'AVVIO ho ricevuto entrambe le risposte alla mia richiesta
                                ESP_LOGI(PING_TAG, "Ping --> OK\n");
                                pingOK = 1;

                                //  ( se esp1 riceve sia errore spiff che errore sd --> esp2 non può funzionare )
                                if (!Spiff_works)       { TWAI_sendError(0x0B); SD_operation(&SD, 1, 1, "ERRORE SPIFF"); }
                                if (!SD_works)          { TWAI_sendError(0x09); SD_operation(&SD, 1, 1, "ERRORE SD"); }
                                if (!Fotocellula_works) { TWAI_sendError(0x00); SD_operation(&SD, 1, 1, "ERRORE FOTOCELLULA"); }
                                if (!I2c_works)         { TWAI_sendError(0x07); SD_operation(&SD, 1, 1, "ERRORE I2C"); }

                                if (SD_works) {
                                    if (!SD_check(&SD, MOUNT_POINT, "values.txt")) {
                                        velocita_impostata = 0;
                                        MSG_valori_in_SD();
                                        ESP_LOGI(PING_TAG, "Ho scritto i valori in SD");
                                    } else {
                                        MSG_read_valori_in_SD();
                                        velocita_impostata = 0;
                                        ESP_LOGI(PING_TAG, "Ho letto i valori in SD\n");
                                    }
                                    t_sd_closed = millis();
                                } else ESP_LOGE(PING_TAG, "SD non funzionante --> non scrivo o leggo da file valori e aspetto valori da esp1");       
                            }
                            break;

                        default:
                            ESP_LOGE("macchina_ready", "Ho ricevuto un messaggio con priorità non accettata in 'macchina_ready()'");
                            break;
                    }
                }
            }
        }

        //  ERRORE PING
        /*if ((millis() - t_pingRequest) > 20000) {
            TWAI_sendError(0x05);
            ESP_LOGE("macchina_ready", "ERRORE PING --> non ho ricevuto 2 pingReply e 2 pingRequest entro 20 secondi");
        }*/

        delay(10);
    }
}


void MSG_message_receiver() {
    if(TWAI_get() == ESP_OK) {
        if(message.identifier != 0) {
            priorita = (message.identifier >> 8) & 0x7;     //  printf("- Priorità --> %u\n", priorita);
            mittente = (message.identifier >> 4) & 0xF;     //  printf("- Mittente --> %u\n", mittente);
            destinatario = message.identifier & 0xF;        //  printf("- Destinatario --> %u\n", destinatario);        printf("\n\n");

            if ((destinatario == 2) || (destinatario == 4))
            {
                switch (priorita) {
                    //  PING
                    case 3:
                        switch (buf[0]) {
                            case PING_REQUEST:
                                ESP_LOGI(PING_TAG, "PING request da esp%d", mittente);      //  RIAVVIO di uno dei due esp
                                TWAI_pingReply(versioneFW, mittente);

                                if (mittente == 1) {                                        //  RIAVVIO DI ESP1 (DISPLAY)
                                    
                                    stato_tartaruga = 0;
                                    stato_pedale = 0;
                                    stato_s1 = 0;

                                    if (stato_motore == 0) motore_ = 0; else motore_ = 1;
                                    if (stato_motore < 4) modalita_ = stato_motore;
                                    if (stato_pedale == 1) pedale_ = 1; else pedale_ = 0;

                                    if (!emergenze_funghi && !emergenze_fotocellule) emergenza_ = 0;
                                    else if (emergenze_funghi && !emergenze_fotocellule) emergenza_ = 1;
                                    else if (!emergenze_funghi && emergenze_fotocellule) emergenza_ = 2;

                                    pid_ = PID_getState(&PID);
                                    if (stato_s1 == 1) stato_s1_ = 0; else stato_s1_ = 1;
                                    if (funzione_s1 == 1) funzione_s1_ = 1; else funzione_s1_ = 0;
                                    //  void TWAI_sendStatoMacchina(uint8_t destinatario, uint8_t motore, uint8_t modalita, uint8_t pedale, uint8_t emergenza, uint8_t pid, uint8_t stato_s1, uint8_t funzione_s1);
                                    TWAI_sendStatoMacchina(mittente, 0/*motore_*/, 0/*modalita_*/, pedale_, emergenza_, pid_, stato_s1_, funzione_s1_);


                                    //stato_pid = 0;  //  PID OFF
                                    PID_stop(&PID);
                                    ESP_LOGI("taskPid", "pid --> OFF");
                                    //PID_scrivi_file_SD("PID OFF");
                                    SD_operation(&SD, 1, 1, "PID OFF");
                                    PID_check_temperatura_attuale(&PID, temperatura_attuale);

                                    if (inizio_pid != 0) millis_pid += millis() - inizio_pid;
                                    inizio_pid = 0;

                                    t_pidoff = millis();
                                
                                }
                                break;

                            case PING_REPLY:
                                ESP_LOGI(PING_TAG, "PING reply da esp%d", mittente);
                                switch (mittente) {
                                    case 1: versioneFW_esp1 = buf[1]; break;
                                    case 3: versioneFW_esp3 = buf[1]; break;
                                    default: ESP_LOGE(PING_TAG, "mittente non riconosciuto"); break;
                                }
                                break;

                            default:
                                ESP_LOGE(PING_TAG, "ERROR --> byte0 = %u", buf[0]);
                                break;
                        }
                        break;

                    //  COMANDI E DATI
                    case 1:
                    case 2:
                        switch (buf[0]) {
                            case MSG_MOTOR:
                                if (mittente == 3) {
                                    if ((buf[1] < 4)/* && (buf[1] != stato_motore)*/) {
                                        last_stato_motore = stato_motore;
                                        stato_motore = buf[1];
                                    }

                                    if(buf[1] == 0) last_stato_motore = 0;

                                    switch (buf[1]) {
                                        case MOTOR_OFF:
                                        case MOTOR_REVERSE:
                                        case MOTOR_TURTLE:
                                            allow_photocell = 0;
                                            break;

                                        case MOTOR_ON:
                                            allow_photocell = 1;

                                            //if (last_stato_motore == 3) {
                                                /*ESP_LOGI(MSG_TAG, "passaggio da modalità tartaruga a modalità manuale con marcia on");
                                                TWAI_sendStatoPedale(0x00, 0x00);
                                                stato_tartaruga = 0;
                                                pedale_premuto = 0;
                                                ultimo_pedale_premuto = pedale_premuto;

                                                //  riattiva fotocellule, lasciando il tasto reset non premuto e riattivando l interrupt
                                                gpio_set_level(GPIO_NUM_48, 1);     //  reset
                                                delay(100);
                                                gpio_set_direction(GPIO_NUM_48, GPIO_MODE_INPUT);
                                                gpio_intr_enable(GPIO_NUM_48);*/
                                                
                                                //stato_pedale = 0;
                                            //}

                                            if (stato_tartaruga && pedale) {
                                                ESP_LOGI("PEDALE", "rilasciato");
                                                TWAI_sendStatoPedale(0x00, 0x00);    //  rilasciato
                                                ESP_LOGI("PEDALE", "PASSA DA TARTARUGA A MODALITA' MANUALE CON MOTORE ON");
                                                TWAI_sendMotorCommand(MOTOR_ON);
                                                allow_photocell = 1;

                                                //  riattiva fotocellule, lasciando il tasto reset non premuto e riattivando l interrupt
                                                gpio_set_level(GPIO_NUM_48, 1);     //  reset
                                                delay(100);
                                                gpio_set_direction(GPIO_NUM_48, GPIO_MODE_INPUT);
                                                gpio_intr_enable(GPIO_NUM_48);
                                                fotoc_dis_da_reset = 0;

                                                stato_tartaruga = 0;
                                            }
                                            
                                            if (!emergenze && fotocellula_interrotta && allow_photocell) {
                                                ESP_LOGI(MSG_TAG, "invio emergenza per fotocellule occupate quando ho ricevuto MOTOR ON");
                                                EM_emergency(0);
                                                twai_clear_transmit_queue();
                                                TWAI_sendEmergenze(gpio_get_level(FOTOC1), gpio_get_level(FOTOC2), gpio_get_level(EM2), gpio_get_level(EM1), 0x01/*!gpio_get_level(EMERGENZE)*/);
                                                n_emergenza = 1;
                                                if (EM_azione_1 != 1) { EM_start_azione(1); EM_azione_1 = 1; }
                                            }

                                            //if (stato_pedale && pedale_premuto) TWAI_sendStatoPedale(0x01, 0x00);

                                            break;

                                        case MOTOR_CURRENT_MAX:
                                            corrente_massima = buf[2];
                                            ESP_LOGI("taskMsg", "corrente massima = %d", corrente_massima);
                                            valori_cambiati = 1;
                                            t_valori_cambiati = millis();
                                            break;

                                        default:
                                            ESP_LOGE("taskMsg", "ERROR --> comando motore (byte 1) ?");
                                            break;
                                    }

                                    if (stato_motore == 0) {
                                        if (inizio_motore != 0) {
                                            millis_motore += millis() - inizio_motore;
                                            inizio_motore = 0;
                                        }
                                    } //else {
                                    
                                    if ((stato_motore > 0) && (stato_motore < 4)) {
                                        if (inizio_motore == 0) inizio_motore = millis();
                                    }

                                    if ((stato_motore == 1) || (stato_motore == 3)) {
                                        gpio_set_level(ENABLE, 1);
                                    } else if ((stato_motore == 0) || (stato_motore == 2)) {
                                        gpio_set_level(ENABLE, 0);
                                    }

                                    /* //PER UTILIZZARE FOTOCELLULA 2 IN REVERSE
                                    if ((stato_motore == 1) || (stato_motore == 3) || (stato_motore == 2)) {
                                        gpio_set_level(ENABLE, 1);
                                    } else if (stato_motore == 0)  {
                                        gpio_set_level(ENABLE, 0);
                                    }
                                    */
                                    
                                }
                                break;

                            case MSG_ELETTROVALVOLE:
                                TWAI_sendConfermaElettrovalvole(buf[1]);
                                switch (buf[1]) {
                                    case 0x00:  //  abbassa --> pulsante 9
                                        if (t_alza >= t_abbassa) {
                                            t_msg_abbassa = millis();
                                            if (allow_abbassa == 1) {
                                                if (gpio_get_level(FOTOC1) == 0) {
                                                    gpio_set_level(ABBASSA, 1); //  OUT 2
                                                    ESP_LOGI("taskMsg", "Abbassa ON");
                                                    t_abbassa = esp_timer_get_time()/1000;    //  0;
                                                    abbassa = 1;
                                                } else {
                                                    ESP_LOGI(MSG_TAG, "invio emergenze per fotocellula occupata quando ho ricevuto ABBASSA");
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
                                                ESP_LOGE("taskMsg", "ABBASSA non riuscito --> emergenze attivate");
                                            }
                                        }
                                        break;

                                    case 0x01:  //  alza --> pulsante 7
                                        if (t_abbassa >= t_alza)/*(((millis() - t_msg_alza) > 1050) || ((millis() - t_abbassa_off) > 50) || ((millis() - t_alza_off) > 50))*/ {
                                            t_msg_alza = millis();
                                            gpio_set_level(ALZA, 1);    //  OUT 1
                                            ESP_LOGI("taskMsg", "Alza ON");
                                            t_alza = esp_timer_get_time()/1000;    //  0;
                                            alza = 1;
                                        }
                                        break;

                                    default:
                                        ESP_LOGE("taskMsg", "ERROR --> stato sconosciuto");
                                        break;
                                }
                                break;

                            case MSG_EMERGENCY:
                                //  conferma ricezione emergenza da esp1
                                break;

                            case MSG_PID:
                                switch (buf[1]) {
                                    case PID_MIN:
                                        if ((buf[2] >= 0) && (buf[2] < temperatura_massima)) {
                                            temperatura_minima = buf[2];
                                            ESP_LOGI("taskMsg", "temperatura minima = %d", temperatura_minima);
                                        } else ESP_LOGE("taskMsg", "ERROR --> temperatura minima out of range");
                                        break;

                                    case PID_MAX:
                                        if ((buf[2] > temperatura_minima) && (buf[2] <= 60)) {
                                            temperatura_massima = buf[2];
                                            ESP_LOGI("taskMsg", "temperatura massima = %d", temperatura_massima);
                                        } else ESP_LOGE("taskMsg", "ERROR --> temperatura massima out of range");
                                        break;

                                    case PID_SET:
                                        if ((millis() - t_pidon) > 100) {
                                            //  INIT WATCHDOG QUANDO PID ON
                                            //PID_init_WDT();
                                            /*if ((millis() - t_watchdog) > 50) {
                                                PID_init_WDT();
                                                t_watchdog = millis();
                                                ESP_LOGI("WATCHDOG", "INIT");
                                            } else {
                                                ESP_LOGE("WATCHDOG", "'PID ON' ARRIVATO DOPO MENO DI 100ms DA 'PID OFF'");
                                                init_or_deinit = 1;
                                            }*/

                                            

                                            if ((buf[2] > temperatura_minima) && (buf[2] <= temperatura_massima)) {
                                                if (allow_pid == true) {

                                                    //
                                                    PID_1_st_ch = !PID_getState(&PID); //accensione da off
                                                    PID_1_st_ch |= buf[2] != target_temperature; // modifica temperatura
                                                    //

                                                    target_temperature = buf[2]; 
                                                    ESP_LOGI("taskMsg", "temperatura impostata = %d", target_temperature);

                                                    //stato_pid = 1;  //  PID ON
                                                    if (!PID_getState(&PID)) {
                                                        
                                                        ESP_LOGI("taskPid", "KP: %f , KI: %f , KD: %f\n", PID.KP, PID.KI, PID.KD);
                                                        PID_start(&PID, target_temperature);
                                                        ESP_LOGI("taskPid", "pid --> ON");
                                                        //PID_scrivi_file_SD("PID ON");
                                                        SD_operation(&SD, 1, 1, "PID ON");
                                                    } else {
                                                        PID_setTemp(&PID, target_temperature);
                                                        ESP_LOGI("taskPid", "target impostato");
                                                        SD_operation(&SD, 1, 1, "NEW TARGET");
                                                    }

                                                    if (inizio_pid == 0) inizio_pid = millis();
                                                    
                                                } else {
                                                    target_temperature = buf[2]; 
                                                    PID_setTemp(&PID, target_temperature);          //  if state==0 -> imposta target , if state==1 -> PID_start con nuovo target
                                                    ESP_LOGE("taskPid", "PID ON non riuscito --> emergenze attivate");

                                                    if (inizio_pid == 0) inizio_pid = millis();
                                                }
                                            } else ESP_LOGE("taskMsg", "ERROR --> temperatura impostata out of range");
                                            PID_check_temperatura_attuale(&PID, temperatura_attuale);

                                            t_pidon = millis();
                                        }
                                        break;

                                    case PID_KP:
                                        kp = buf[2] / 10.0;
                                        ESP_LOGI("taskMsg", "kp = %1.1f", kp);
                                        PID_setPidParameters(&PID, kp, ki, kd);
                                        break;

                                    case PID_KI:
                                        ki = buf[2] / 100.0;
                                        ESP_LOGI("taskMsg", "ki = %1.1f", ki);
                                        PID_setPidParameters(&PID, kp, ki, kd);
                                        break;

                                    case PID_KD:
                                        kd = buf[2] / 10.0;
                                        ESP_LOGI("taskMsg", "kd = %1.1f", kd);
                                        PID_setPidParameters(&PID, kp, ki, kd);
                                        break;

                                    case PID_OFFSET:
                                        offset = buf[2] /*/ 10.0*/;
                                        ESP_LOGI("taskMsg", "offset = %1.1f", offset);
                                        PID_setOffset(&PID, offset);
                                        if (buf[3] == 0x00) { segno_offset = 0; ESP_LOGI("taskMsg", "segno offset = +"); } 
                                        else if (buf[3] == 0x01) { segno_offset = 1; ESP_LOGI("taskMsg", "segno offset = -"); }
                                        break;

                                    case PID_OFF:
                                        if ((millis() - t_pidoff) > 100) {
                                            //  DEINIT WATCHDOG QUANDO PID OFF
                                            //PID_deinit_WDT();
                                            /*if ((millis() - t_watchdog) > 50) {
                                                PID_deinit_WDT();
                                                t_watchdog = millis();
                                                ESP_LOGI("WATCHDOG", "DEINIT");
                                            } else {
                                                ESP_LOGE("WATCHDOG", "'PID OFF' ARRIVATO DOPO MENO DI 100ms DA 'PID ON'");
                                                init_or_deinit = 2;
                                            }*/

                                            PID_1_st_ch = PID_getState(&PID); //spegnimento da on
                                            //  PID_1_st_ch |= buf[2] != target_temperature; // modifica temperatura

                                            //stato_pid = 0;  //  PID OFF
                                            PID_stop(&PID);
                                            ESP_LOGI("taskPid", "pid --> OFF");
                                            //PID_scrivi_file_SD("PID OFF");
                                            SD_operation(&SD, 1, 1, "PID OFF");
                                            PID_check_temperatura_attuale(&PID, temperatura_attuale);

                                            if (inizio_pid != 0) millis_pid += millis() - inizio_pid;
                                            inizio_pid = 0;

                                            t_pidoff = millis();
                                        }
                                        break;
                                    
                                    default:
                                        ESP_LOGE("taskMsg", "ERROR --> impostazione temperatura (byte 1) ?");
                                        break;
                                }
                                if (buf[1] < 7) {
                                    valori_cambiati = 1;
                                    t_valori_cambiati = millis();
                                }
                                break;

                            case MSG_MOTOR_SPEED:
                                TWAI_sendConfermaVelocita(buf[1], buf[2]);
                                switch (buf[1]) {
                                    case SPEED_MIN:
                                        velocita_minima = buf[2];
                                        ESP_LOGI("taskMsg", "velocità minima = %d", velocita_minima);
                                        break;

                                    case SPEED_MAX:
                                        velocita_massima = buf[2];
                                        ESP_LOGI("taskMsg", "velocità massima = %d", velocita_massima);
                                        break;

                                    case SPEED_REV:
                                        if ((buf[2] >= velocita_minima) && (buf[2] <= velocita_massima)) {
                                            velocita_reverse = buf[2];
                                            ESP_LOGI("taskMsg", "velocità reverse = %d", velocita_reverse);
                                        } else ESP_LOGE("taskMsg", "ERROR --> velocita reverse out of range");
                                        break;

                                    case SPEED_SET:
                                        if ((buf[2] >= velocita_minima) && (buf[2] <= velocita_massima)) {
                                            velocita_impostata = buf[2];
                                            ESP_LOGI("taskMsg", "velocità impostata a %d", velocita_impostata);
                                        } else ESP_LOGE("taskMsg", "ERROR --> velocita impostata out of range");
                                        break;
                                    
                                    case 0x04:
                                        if ((buf[2] >= velocita_minima) && (buf[2] <= velocita_massima)) {
                                            velocita_tartaruga = buf[2];
                                            ESP_LOGI("startCheck", "velocità tartaruga = %d", velocita_tartaruga);
                                        } else ESP_LOGE("startCheck", "ERROR --> velocita tartaruga out of range");
                                        break;
                                    
                                    default:
                                        ESP_LOGE("taskMsg", "ERROR --> impostazione velocita (byte 1) ?");
                                        break;
                                }
                                if (buf[1] < 5) {
                                    valori_cambiati = 1;
                                    t_valori_cambiati = millis();
                                }
                                break;

                            case MSG_PEDAL:
                                /*if (gpio_get_level(PEDALE_PIN) == 0) {
                                    pedale = 1;
                                    ped_pre_al_pass_mod_ped = 1;
                                } else {
                                    pedale = 0;
                                    ESP_LOGI("PEDALE", "SPEGNI IL MOTORE");
                                    TWAI_sendMotorCommand(MOTOR_OFF);
                                }

                                //  TWAI_sendConfermaPedale();

                                pedale_premuto = 0;
                                ultimo_pedale_premuto = 0;*/

                                TWAI_sendConfermaPedale();

                                switch (buf[1]) {
                                    case 0:
                                        if (stato_pedale && pedale) {
                                            ESP_LOGI("PEDALE", "rilasciato");
                                            TWAI_sendStatoPedale(0x01, 0x00);  //  rilasciato
                                        }

                                        stato_pedale = 0;

                                        break;

                                    case 1:
                                        stato_pedale = 1;
                                        stato_tartaruga = 0;
                                        stato_s1 = 0;
                                        funzione_premuto = 1;
                                        funzione_rilasciato = 0;

                                        //  STOP MOTORE QUANDO ENTRA IN MODALITA' PEDALE
                                        if (!pedale) {
                                            ESP_LOGI("PEDALE", "SPEGNI IL MOTORE");
                                            TWAI_sendMotorCommand(MOTOR_OFF);
                                            da_man_a_ped = 1;
                                        }
                                        break;
                                }
                                break;

                            case MSG_TURTLE:
                                TWAI_sendConfermaTurtle();
                                switch (buf[1]) {
                                    case TURTLE_OFF:
                                        /*if (stato_tartaruga && pedale) {    //  motore parte sempre quando viene tolta modalità tartaruga
                                            TWAI_sendMotorCommand(MOTOR_ON);
                                            TWAI_sendStatoPedale(0x00, 0x00);
                                        }*/

                                        if (stato_tartaruga && pedale) {
                                            ESP_LOGI("PEDALE", "rilasciato");
                                            TWAI_sendStatoPedale(0x00, 0x00);    //  rilasciato
                                            ESP_LOGI("PEDALE", "SPEGNI IL MOTORE");
                                            TWAI_sendMotorCommand(MOTOR_OFF);
                                            allow_photocell = 0;

                                            //  riattiva fotocellule, lasciando il tasto reset non premuto e riattivando l interrupt
                                            gpio_set_level(GPIO_NUM_48, 1);     //  reset
                                            delay(100);
                                            gpio_set_direction(GPIO_NUM_48, GPIO_MODE_INPUT);
                                            gpio_intr_enable(GPIO_NUM_48);
                                            fotoc_dis_da_reset = 0;
                                        }

                                        stato_tartaruga = 0;

                                        break;

                                    case TURTLE_ON:
                                        //  per passare con pedale premuto, da pedale a motor on e poi a tartaruga con il pedale sempre premuto
                                        
                                        //ped_pre_al_pass_mod_ped = 0;
                                        /*if (gpio_get_level(PEDALE_PIN) == 0) {
                                            pedale = 1;
                                            ped_pre_al_pass_mod_ped = 1;
                                        }*/
                                        //ultimo_pedale_premuto = 0;
                                        //pedale_premuto = 0;

                                        stato_tartaruga = 1;
                                        stato_pedale = 0;
                                        stato_s1 = 0;
                                        funzione_premuto = 1;
                                        funzione_rilasciato = 0;
                                        break;
                                }
                                break;

                            case MSG_S1:
                                TWAI_sendConfermaS1();
                                if (buf[1] == 1) funzione_s1 = 1; else funzione_s1 = 0;
                                if (buf[2] == 1) {
                                    stato_s1 = 1;
                                    stato_tartaruga = 0;
                                    stato_pedale = 0;
                                    funzione_premuto = 1;
                                    funzione_rilasciato = 0;
                                } else stato_s1 = 0;
                                break;

                            case MSG_AZ_EM:
                                break;

                            case MSG_ORE:
                                risposta = buf[1];
                                switch (buf[1]) {
                                    case 0x00:      //  VISUALIZZA ORE DI UTILIZZO IN SCHERMATA DEDICATA
                                        millisToOreMsbLsb();
                                        ESP_LOGI("INVIO ORE", "ore_pid_MSB, ore_pid_LSB, ore_mot_MSB, ore_mot_LSB, ore_utilizzo_mac_MSB, ore_utilizzo_mac_LSB");
                                        TWAI_sendOreUtilizzo(risposta, ore_pid_MSB, ore_pid_LSB, ore_mot_MSB, ore_mot_LSB, ore_utilizzo_mac_MSB, ore_utilizzo_mac_LSB);
                                        break;

                                    case 0x01:      //  ESPORTA FILE DA SPIFF IN SD (verranno visualizzati i minuti)
                                        SD_update_statistics();
                                        TWAI_sendConfermaEsportaOreInSd();
                                        break;

                                    case 0x02:      //  RESET CONTATORI (millis = 0)
                                        millis_macchina = 0;
                                        millis_pid = 0;
                                        millis_motore = 0;

                                        //  reset inizio millis
                                        inizio_macchina = millis();
                                        if (PID_getState(&PID) == 1) inizio_pid = millis();
                                        if (stato_motore != 0) inizio_motore = millis();

                                        ready = 0;
                                        while(!ready) {
                                            if ((millis() - t_statistiche_in_SPIFF) > 100) {
                                                if (SPIFFS.busy == 0) {
                                                    SPIFFS_write_(&SPIFFS);
                                                    t_statistiche_in_SPIFF = millis();
                                                    ESP_LOGI(MSG_TAG, "Contatori azzerati e scritti in SPIFF");
                                                    ready = 1;
                                                } else {
                                                    ESP_LOGE("scrittura spiff per reset contatori", "SPIFF busy in un'altra operazione, riprovo tra altri 3 secondi");
                                                    //t_statistiche_in_SPIFF = millis();
                                                }
                                            }
                                        }
                                        
                                        millisToOreMsbLsb();
                                        TWAI_sendOreUtilizzo(risposta, ore_pid_MSB, ore_pid_LSB, ore_mot_MSB, ore_mot_LSB, ore_utilizzo_mac_MSB, ore_utilizzo_mac_LSB);
                                        //ESP_LOGE("-", "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO");
                                        break;
                                }
                                break;

                            case MSG_VAL_INIT:
                                switch (priorita) {
                                    case 1: //  comando / richiesta
                                        //if (valori_cambiati) MSG_valori_in_SD();
                                        if (valori_cambiati) {
                                            while(valori_cambiati) {
                                                if ((millis() - t_valori_cambiati/*t_update_values_in_SD*/) > 3000) {
                                                    MSG_valori_in_SD();
                                                    /*t_update_values_in_SD*/ t_valori_cambiati = millis();
                                                }
                                                delay(10);
                                            }
                                        }
                                        MSG_read_valori_in_SD();
                                        TWAI_sendValoriMotore(mittente, velocita_minima, velocita_massima, velocita_reverse, velocita_tartaruga, velocita_impostata, corrente_massima);
                                        if (mittente == 1) {
                                            TWAI_sendValoriRiscaldamento1(temperatura_minima, temperatura_massima, target_temperature);
                                            kp_ = kp*10;   ki_ = ki*100;   kd_ = kd*10;   offset_ = offset/**10*/;
                                            if (segno_offset) segno_offset_ = 1; else segno_offset_ = 0;
                                            TWAI_sendValoriRiscaldamento2(kp_, ki_, kd_, offset_, segno_offset_);
                                        }
                                        break;

                                    case 2: //  dati / risposta
                                        switch(buf[1]) {
                                            case 0:
                                                velocita_minima = buf[2];
                                                velocita_massima = buf[3];
                                                velocita_reverse = buf[4];
                                                velocita_tartaruga = buf[5];
                                                velocita_impostata = buf[6];
                                                corrente_massima = buf[7];
                                                break;
                                            
                                            case 1:
                                                temperatura_minima = buf[2];
                                                temperatura_massima = buf[3];
                                                target_temperature = buf[4];
                                                break;

                                            case 2:
                                                kp = buf[2]/10.0;
                                                ki = buf[3]/100.0;
                                                kd = buf[4]/10.0;
                                                offset = buf[5]/*/10.0*/;
                                                if (buf[6] == 0x00) segno_offset = 0; else if (buf[6] == 0x01) segno_offset = 1;
                                                break;
                                        }
                                        MSG_valori_in_SD();
                                        break;
                                }
                                break;

                            case MSG_UPDATES:
                                ESP_LOGI("MSG_RECEIVER", "update");
                                
                                //if (idx == 0x114) {
                                    switch (buf[1]) {
                                        case 0:
                                            ESP_LOGI(MSG_TAG, "Check update");
                                            if(checkupdate()){
                                                Version* firmwareVersion = getFirmwareVersions();
                                                uint32_t total_files_size= getFileSize("/sdcard/updates/Display/firmware.bin") + getFileSize("/sdcard/updates/Driver/firmware.bin") ;
                                                ESP_LOGI(MSG_TAG, "total_files_size %ld",total_files_size);

                                                TWAI_sendUpdateSize(total_files_size);
                                                ESP_LOGI(MSG_TAG, "SEND message UPDATE AVAILABLE with version");
                                                TWAI_sendUpdateInfo(firmwareVersion);
                                                
                                                ESP_LOGI(MSG_TAG, "SEND update info also");
                                                
                                            } else {
                                                ESP_LOGI(MSG_TAG, "SEND message NO update");
                                            }
                                            
                                            break;
                                        /*
                                        case 1:
                                            ESP_LOGI(msg_tag, "SEND message NO update");
                                            break;

                                        case 2:
                                            ESP_LOGI(msg_tag, "SEND message UPDATE AVAILABLE with version");
                                            ESP_LOGI(msg_tag, "SEND update info also");
                                            break;
                                        */
                                    
                                        case 3:
                                            //  DEINIT WATCHDOG QUANDO PID OFF
                                            PID_deinit_WDT();

                                            if (buf[2]==0) {
                                                ESP_LOGI(MSG_TAG, "Anbort or CAncel Update");
                                                break;
                                            } else if (buf[2]==1) {
                                                ESP_LOGI(MSG_TAG, "Update comnfirmed call performUpdate() 1");
                                                //TODO cancell update

                                                perform_update = 1;

                                                /*bool res = performUpdate();
                                                ESP_LOGI(MSG_TAG, "Update comnfirmed call performUpdate() 2");
                                                esp_restart();*/
                                                
                                                //vTaskSuspend(globalTaskHandler);
                                                //vTaskSuspend(globalTaskHandler_2);

                                                //  perform_update = 0;     //  --> per messaggi di riavvio
                                                

                                                break;
                                            }
                                        
                                        default:
                                            //return -1;
                                            break;
                                        
                                    }
                               // }
                                break;
                            
                            default:
                                //ESP_LOGE("taskMsg", "ERROR --> tipologia messaggio (byte 0) ?");
                                break;
                        }
                        break;

                    case 0:
                        ESP_LOGE("Tipo messaggio -->", "Priorità = ERRORE\n");
                        switch (buf[0]) {
                            case 0x02:
                                ESP_LOGE("ERROR", "Errore di superamento corrente di trazione (esp%u)", mittente);  //  esp3
                                break;

                            case 0x03:
                                ESP_LOGE("ERROR", "Errore sensore di temperatura driver motore (esp%u)", mittente);     //  esp3
                                break;

                            case 0x04:
                                ESP_LOGE("ERROR", "Errore relè driver motore (esp%u)", mittente);   //  esp3
                                break;

                            case 0x07:
                                ESP_LOGE("ERROR", "Errore I2c (esp%u)", mittente);  //  esp1
                                break;

                            case 0x0B:
                                ESP_LOGE("ERROR", "Errore spiff (esp%u)", mittente);    //  esp1 / esp3?
                                break;
                        }
                        break;

                    default:
                        ESP_LOGE("taskMsg", "ERROR --> priorità sconosciuta");
                        break;
                }
            }
        }
    } //else ESP_LOGI(msg_tag,"...");
    memset(buf, 0, LEN);
    message.identifier = 0;
}
    