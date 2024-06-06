#include "TWAI.h"

//  FUNZIONI BASE
esp_err_t TWAI_init() {
    //Initialize configuration structures using macro initializers
    static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    static const uint32_t acceptanceMask = ~(0x00F << 21);
    /*static const twai_filter_config_t f_config = {  .acceptance_code = (0x2 << 21),     //  byte da accettare
                                                    .acceptance_mask = acceptanceMask,    //  maschera il primo byte (partendo da destra) --> definisci il destinatario
                                                    .single_filter = true   };  //  TWAI_FILTER_CONFIG_ACCEPT_ALL();    */
    static const twai_filter_config_t filter_configs[] = {
        {
            .acceptance_code = (0x002 << 21),   // Set the acceptance code (ID to match ending with '1')
            .acceptance_mask = acceptanceMask,  // Set the acceptance mask to allow only lower 4 bits
            .single_filter = false      // Use multiple filters
        },
        {
            .acceptance_code = (0x004 << 21),   // Set the acceptance code (ID to match ending with '6')
            .acceptance_mask = acceptanceMask,  // Set the acceptance mask to allow only lower 4 bits
            .single_filter = false      // Use multiple filters
        }
    };
    static const twai_general_config_t g_config = {
                                                    .mode = TWAI_MODE_NORMAL,
                                                    .tx_io = (gpio_num_t)ESP_2_TX,
                                                    .rx_io = (gpio_num_t)ESP_2_RX,
                                                    .clkout_io = TWAI_IO_UNUSED,
                                                    .bus_off_io = TWAI_IO_UNUSED,
                                                    .tx_queue_len = 10000,  //32, //  32?
                                                    .rx_queue_len = 10000,  //32, //  32?
                                                    .alerts_enabled = TWAI_ALERT_ALL,   //TWAI_ALERT_NONE,
                                                    .clkout_divider = 0,
                                                    .intr_flags = ESP_INTR_FLAG_LEVEL1
                                                };  // TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)ESP_2_TX, (gpio_num_t)ESP_2_RX, TWAI_MODE_NORMAL);

    //Install TWAI driver
    //if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    if (twai_driver_install(&g_config, &t_config, filter_configs) == ESP_OK) {
        ESP_LOGI(TWAI_TAG, "Driver installed");
    } else {
        ESP_LOGE(TWAI_TAG, "Failed to install driver");
        return ESP_FAIL;
    }

    //Start TWAI driver
    if (twai_start() == ESP_OK) {
        ESP_LOGI(TWAI_TAG, "Driver started");
    } else {
        ESP_LOGE(TWAI_TAG, "Failed to start driver");
        return ESP_FAIL;
    }

    // Reconfigure alerts to detect frame receive, Bus-Off error and RX queue full states
    uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL;
    if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) {
        ESP_LOGI(TWAI_TAG, "CAN Alerts reconfigured");
    } else {
        ESP_LOGE(TWAI_TAG, "Failed to reconfigure alerts");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t TWAI_reinit() {
    //CAN.flush();  //  --> ????

    if (twai_clear_transmit_queue() == ESP_OK) {
        ESP_LOGI(TWAI_TAG, "Transmit queue cleared\n");
    } else {
        ESP_LOGE(TWAI_TAG, "Failed to clear transmit queue --> TWAI driver is not installed or TX queue is disabled\n");
        return ESP_FAIL;
    }

    if (twai_clear_receive_queue() == ESP_OK) {
        ESP_LOGI(TWAI_TAG, "Receive queue cleared\n");
    } else {
        ESP_LOGE(TWAI_TAG, "Failed to clear receive queue --> TWAI driver is not installed\n");
        return ESP_FAIL;
    }

    if (twai_stop() == ESP_OK) {
        ESP_LOGI(TWAI_TAG, "Driver stopped\n");
    } else {
        ESP_LOGE(TWAI_TAG, "Failed to stop driver\n");
        return ESP_FAIL;
    }

    if (twai_driver_uninstall() == ESP_OK) {
        ESP_LOGI(TWAI_TAG, "Driver uninstalled\n");
    } else {
        ESP_LOGE(TWAI_TAG, "Failed to uninstall driver\n");
        return ESP_FAIL;
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);

    if (TWAI_init() == ESP_OK) {
        ESP_LOGI(TWAI_TAG, "TWAI reinit DONE\n");
        return ESP_OK;
    } else {
        ESP_LOGE(TWAI_TAG, "TWAI reinit ERROR\n");
        return ESP_FAIL;
    }
}

esp_err_t TWAI_send(uint32_t id, uint16_t flag, uint8_t len, uint8_t data[]) {          //  ID(scheda che riceve) - FLAG(tipo di messaggio(es. trasmesso/ricevuto)) - LEN - DATA
    //Queue message for transmission
    twai_message_t message;
    message.identifier = id;                // TWAI_STD_ID_MASK;                TWAI_EXTD_ID_MASK;
    message.flags = flag;                   // TWAI_MSG_FLAG_NONE;              TWAI_MSG_FLAG_EXTD;
    message.data_length_code = len;         // TWAI_FRAME_STD_ID_LEN_BYTES;     TWAI_FRAME_EXTD_ID_LEN_BYTES;

    printf("ID: %#lx ", (unsigned long)message.identifier);

    for ( int i = 0 ; i < len ; i++ ) {
        message.data[i] = data[i];
        printf(" %02x ", message.data[i]);
    }

    printf("--> ");

    //Queue message for transmission
    if (twai_transmit(&message, 0) == ESP_OK) {
        ESP_LOGI(TWAI_TAG, "Sent");   //  printf("Message queued for transmission\n");
        return ESP_OK;
    } else {
        ESP_LOGE(TWAI_TAG, "Failed");   //  printf("Failed to queue message for transmission\n");
        return ESP_FAIL;
    }
}

esp_err_t TWAI_send_interrupt(uint32_t id, uint16_t flag, uint8_t len, uint8_t data[]) {          //  ID(scheda che riceve) - FLAG(tipo di messaggio(es. trasmesso/ricevuto)) - LEN - DATA
    twai_message_t message;
    message.identifier = id;                // TWAI_STD_ID_MASK;                TWAI_EXTD_ID_MASK;
    message.flags = flag;                   // TWAI_MSG_FLAG_NONE;              TWAI_MSG_FLAG_EXTD;
    message.data_length_code = len;         // TWAI_FRAME_STD_ID_LEN_BYTES;     TWAI_FRAME_EXTD_ID_LEN_BYTES;

    for ( int i = 0 ; i < len ; i++ ) {
        message.data[i] = data[i];
    }

    if (twai_transmit(&message, 0) == ESP_OK) return ESP_OK; else return ESP_FAIL;
}

esp_err_t TWAI_get() {
    uint32_t alerts_triggered;
    
    twai_read_alerts(&alerts_triggered, 0 /*pdMS_TO_TICKS(0)*/);
    twai_status_info_t twaistatus;
    twai_get_status_info(&twaistatus);

    // Handle alerts
    if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
        ESP_LOGE(TWAI_TAG, "Alert: TWAI controller has become error passive.\n");
    }
    if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
        ESP_LOGE(TWAI_TAG, "Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.\n");
        ESP_LOGE(TWAI_TAG, "Bus error count: %ld\n", twaistatus.bus_error_count);
    }
    if (alerts_triggered & TWAI_ALERT_RX_QUEUE_FULL) {
        ESP_LOGE(TWAI_TAG, "Alert: The RX queue is full causing a received frame to be lost.\n");
        ESP_LOGE(TWAI_TAG, "RX buffered: %ld\t", twaistatus.msgs_to_rx);
        ESP_LOGE(TWAI_TAG, "RX missed: %ld\t", twaistatus.rx_missed_count);
        ESP_LOGE(TWAI_TAG, "RX overrun %ld\n", twaistatus.rx_overrun_count);
    }

    // Check if message is received
    //if (alerts_triggered & TWAI_ALERT_RX_DATA) {
        //Wait for message to be received
    twai_message_t message;
    esp_err_t rcv = twai_receive(&message, 0);
    if (rcv == ESP_OK) {
        //  printf("\nMessage received\n\n");
        TWAI_handle_rx_message(message);
        return ESP_OK;
    } else if (rcv != ESP_ERR_TIMEOUT){
        ESP_LOGE(TWAI_TAG, "Failed to receive message\n\n");
    }
    //}

    return ESP_FAIL;
}

static void TWAI_handle_rx_message(twai_message_t m) {
    message = m;

    printf("ID: %lx [ ", message.identifier);
    idx = message.identifier;
    if (!(message.flags & TWAI_MSG_FLAG_RTR)) {
        for (int i = 0; i < message.data_length_code; i++) {
            printf("%02x ", message.data[i]);
            buf[i] = message.data[i];
        }

        //memcpy(&buf, message.data, LEN);

        printf("]\n");
    }
}

//  FUNZIONI INVIO COMANDI  ----------------------------------------------------------------------------------------------------------------------------------------------------------

void TWAI_pingRequest(uint8_t destinatario) {
    uint32_t *id = PING_ID | ESP2_M_ID | destinatario;
    uint8_t len = 1;
    uint8_t buffer[len];
    buffer[0] = PING_REQUEST;
    TWAI_send(id, FLAG, len, buffer);   //  TWAI_send_interrupt
}

void TWAI_pingReply(uint16_t versioneFW, uint8_t destinatario) {
    uint8_t *id = PING_ID | ESP2_M_ID | destinatario;
    uint8_t len = 2;
    uint8_t buffer[len];
    buffer[0] = PING_REPLY;
    buffer[1] = versioneFW;
    TWAI_send(id, FLAG, len, buffer);   //  TWAI_send_interrupt
}

void TWAI_sendStatoPid(uint8_t stato, uint8_t temperatura) {
    uint8_t *id = DATA_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 3;
    uint8_t buffer[len];
    buffer[0] = MSG_PID;
    buffer[1] = stato;          //  0x00 off - 0x01 on
    buffer[2] = temperatura;    //  valore temperatura attuale
    TWAI_send(id, FLAG, len, buffer);
}

void TWAI_sendOreUtilizzo(uint8_t risposta, uint8_t ore_riscaldamento_MSB, uint8_t ore_riscaldamento_LSB, uint8_t ore_motore_MSB, uint8_t ore_motore_LSB, uint8_t ore_utilizzo_macchina_MSB, uint8_t ore_utilizzo_macchina_LSB) {
    uint8_t *id = DATA_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 8;
    uint8_t buffer[len];
    buffer[0] = MSG_ORE;
    buffer[1] = risposta;
    buffer[2] = ore_riscaldamento_MSB;
    buffer[3] = ore_riscaldamento_LSB; 
    buffer[4] = ore_motore_MSB; 
    buffer[5] = ore_motore_LSB;
    buffer[6] = ore_utilizzo_macchina_MSB;
    buffer[7] = ore_utilizzo_macchina_LSB;
    TWAI_send(id, FLAG, len, buffer);   //  TWAI_send_interrupt
}

void TWAI_sendConfermaEsportaOreInSd() {
    uint8_t *id = DATA_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 2;
    uint8_t buffer[len];
    buffer[0] = MSG_ORE;
    buffer[1] = 0x01;
    TWAI_send(id, FLAG, len, buffer);
}

void TWAI_sendAzioneEmergenze(uint8_t stato, uint8_t azione) {
    uint8_t *id = COMMAND_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 3;
    uint8_t buffer[len];
    buffer[0] = MSG_AZ_EM;
    buffer[1] = stato;  //  0 libera            -   1 interrotta
    buffer[2] = azione; //  0 blink led rosso   -   1 buzzer        -   2 buzzer veloce
    TWAI_send(id, FLAG, len, buffer);   //  TWAI_send_interrupt
}

void TWAI_sendAzioneEmergenze_interrupt(uint8_t stato, uint8_t azione) {
    uint8_t *id = COMMAND_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 3;
    uint8_t buffer[len];
    buffer[0] = MSG_AZ_EM;
    buffer[1] = stato;  //  0 libera            -   1 interrotta
    buffer[2] = azione; //  0 blink led rosso   -   1 buzzer        -   2 buzzer veloce
    TWAI_send_interrupt(id, FLAG, len, buffer);   //  TWAI_send_interrupt
}

void TWAI_sendStatoPedale(uint8_t stato, uint8_t azioni) {
    uint8_t *id = DATA_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 3;
    uint8_t buffer[len];
    buffer[0] = MSG_PEDAL;
    buffer[1] = stato;      //  0x00 modalità manuale  ,  0x01 modalità pedale
    buffer[2] = azioni;     //  0x00 al rilascio  ,  0x01 se premuto
    TWAI_send(id, FLAG, len, buffer);   //  TWAI_send_interrupt
}

void TWAI_sendConfermaPedale() {
    uint8_t *id = DATA_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 2;
    uint8_t buffer[len];
    buffer[0] = MSG_PEDAL;
    buffer[1] = 0x02;
    TWAI_send(id, FLAG, len, buffer);   //  TWAI_send_interrupt
}

void TWAI_sendConfermaTurtle() {
    uint8_t *id = DATA_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 1;
    uint8_t buffer[len];
    buffer[0] = MSG_TURTLE;
    TWAI_send(id, FLAG, len, buffer);   //  TWAI_send_interrupt
}

void TWAI_sendConfermaS1() {
    uint8_t *id = DATA_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 1;
    uint8_t buffer[len];
    buffer[0] = MSG_S1;
    TWAI_send(id, FLAG, len, buffer);   //  TWAI_send_interrupt
}

void TWAI_sendConfermaElettrovalvole(uint8_t azione) {
    uint8_t *id = DATA_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 2;
    uint8_t buffer[len];
    buffer[0] = MSG_ELETTROVALVOLE;
    buffer[1] = azione;      //  0x00 abbassa  ,  0x01 alza
    TWAI_send(id, FLAG, len, buffer);   //  TWAI_send_interrupt
}

void TWAI_sendEmergenze(uint8_t fotocellula1, uint8_t fotocellula2, uint8_t fungo1, uint8_t fungo2, uint8_t pin_emergenze) {
    uint8_t *id = COMMAND_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 6;
    uint8_t buffer[len];
    buffer[0] = MSG_EMERGENCY;
    buffer[1] = fotocellula1;
    buffer[2] = fotocellula2; 
    buffer[3] = fungo1; 
    buffer[4] = fungo2;
    buffer[5] = pin_emergenze;
    TWAI_send_interrupt(id, FLAG, len, buffer);
}

void TWAI_sendRichiestaValori(uint8_t tipo_valori) {
    uint8_t *id = COMMAND_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 2;
    uint8_t buffer[len];
    buffer[0] = MSG_VAL_INIT;
    buffer[1] = tipo_valori;    //  0x00 valori motore  ,  0x01 valori pid  ,  0x02 tutti i valori
    TWAI_send(id, FLAG, len, buffer);   //  TWAI_send_interrupt
}

void TWAI_sendValoriMotore(uint8_t destinatario, uint8_t v_min, uint8_t v_max, uint8_t v_reverse, uint8_t v_turtle, uint8_t v_impostata, uint8_t corrente_max) {
    uint8_t *id = DATA_ID | ESP2_M_ID | destinatario;
    uint8_t len = 8;
    uint8_t buffer[len];
    buffer[0] = MSG_VAL_INIT;
    buffer[1] = 0x00;
    buffer[2] = v_min; 
    buffer[3] = v_max; 
    buffer[4] = v_reverse;
    buffer[5] = v_turtle;
    buffer[6] = v_impostata;
    buffer[7] = corrente_max;
    TWAI_send(id, FLAG, len, buffer);
}

void TWAI_sendValoriRiscaldamento1(uint8_t t_min, uint8_t t_max, uint8_t t_target) {
    uint8_t *id = DATA_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 5;
    uint8_t buffer[len];
    buffer[0] = MSG_VAL_INIT;
    buffer[1] = 0x01;
    buffer[2] = t_min; 
    buffer[3] = t_max; 
    buffer[4] = t_target;
    TWAI_send(id, FLAG, len, buffer);
}

void TWAI_sendValoriRiscaldamento2(uint8_t kp_, uint8_t ki_, uint8_t kd_, uint8_t offset_, uint8_t segno_offset_) {
    uint8_t *id = DATA_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 7;
    uint8_t buffer[len];
    buffer[0] = MSG_VAL_INIT;
    buffer[1] = 0x02;
    buffer[2] = kp_; 
    buffer[3] = ki_; 
    buffer[4] = kd_;
    buffer[5] = offset_;
    buffer[6] = segno_offset_;
    TWAI_send(id, FLAG, len, buffer);
}

void TWAI_sendConfermaVelocita(uint8_t tipo_velocita, uint8_t valore) {
    uint8_t *id = DATA_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 3;
    uint8_t buffer[len];
    buffer[0] = MSG_MOTOR_SPEED;
    buffer[1] = tipo_velocita;      //  0 min - 1 max - 2 reverse - 3 impostata - 4 turtle
    buffer[2] = valore;
    TWAI_send(id, FLAG, len, buffer);
}

void TWAI_sendMotorCommand(uint8_t comando) {
    uint8_t *id = COMMAND_ID | ESP2_M_ID | ESP3_D_ID;
    uint8_t len = 2;
    uint8_t buffer[len];
    buffer[0] = MSG_MOTOR;
    buffer[1] = comando;    //  MOTOR_OFF - MOTOR_ON - MOTOR_REVERSE - MOTOR_TURTLE
    TWAI_send(id, FLAG, len, buffer);
}

void TWAI_sendError(uint8_t error) {
    uint8_t *id = ERROR_ID | ESP2_M_ID | BROADCAST_D_ID;
    uint8_t len = 1;
    uint8_t buffer[len];
    buffer[0] = error;
    TWAI_send(id, FLAG, len, buffer);   //  TWAI_send_interrupt
}

void TWAI_sendStatoMacchina(uint8_t destinatario, uint8_t motore, uint8_t modalita, uint8_t pedale, uint8_t emergenza, uint8_t pid, uint8_t stato_s1, uint8_t funzione_s1) {     //  risposta al riavvio di un altro esp
    uint8_t *id = DATA_ID | ESP2_M_ID | destinatario;
    uint8_t len = 8;
    uint8_t buffer[len];
    buffer[0] = 0x00;
    buffer[1] = motore;
    buffer[2] = modalita; 
    buffer[3] = pedale; 
    buffer[4] = emergenza;
    buffer[5] = pid;
    buffer[6] = stato_s1;
    buffer[7] = funzione_s1;
    TWAI_send(id, FLAG, len, buffer);   //  TWAI_send_interrupt
}

void TWAI_sendReset() {
    uint8_t *id = COMMAND_ID | ESP2_M_ID | ESP3_D_ID;
    uint8_t len = 2;
    uint8_t buffer[len];
    buffer[0] = MSG_EMERGENCY;
    buffer[1] = 0x02;
    TWAI_send_interrupt(id, FLAG, len, buffer);
}
