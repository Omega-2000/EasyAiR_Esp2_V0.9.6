#include "MSG_LISTENER.h"

//  v_min;v_max;v_rev;v_tar;v_imp;c_max;t_min;t_max;t_imp;kp;ki;kd;offset;segno_offset
void MSG_valori_in_SD() {
    //  VALORI MOTORE
    char char_v_min[5];     snprintf(char_v_min, sizeof(char_v_min), "%d", velocita_minima);        //SD.v_min = velocità_minima;
    char char_v_max[5];     snprintf(char_v_max, sizeof(char_v_max), "%d", velocita_massima);       //SD.v_max = velocità_massima;
    char char_v_rev[5];     snprintf(char_v_rev, sizeof(char_v_rev), "%d", velocita_reverse);       //SD.v_rev = velocità_reverse;
    char char_v_tar[5];     snprintf(char_v_tar, sizeof(char_v_tar), "%d", velocita_tartaruga);     //SD.v_tar = velocità_tartaruga;
    char char_v_imp[5];     snprintf(char_v_imp, sizeof(char_v_imp), "%d", velocita_impostata);     //SD.v_imp = velocità_impostata;
    char char_c_max[5];     snprintf(char_c_max, sizeof(char_c_max), "%d", corrente_massima);       //SD.c_max = corrente_massima;
    //  VALORI PID 1
    char char_t_min[5];     snprintf(char_t_min, sizeof(char_t_min), "%d", temperatura_minima);     //SD.t_min = temperatura_minima;
    char char_t_max[5];     snprintf(char_t_max, sizeof(char_t_max), "%d", temperatura_massima);    //SD.t_max = temperatura_massima;
    char char_t_imp[5];     snprintf(char_t_imp, sizeof(char_t_imp), "%d", target_temperature);     //SD.t_imp = target_temperature;
    //  VALORI PID 2
    int kp_ = kp*10;   int ki_ = ki*100;   int kd_ = kd*10;   int offset_ = offset/**10*/;
    int segno_offset_; 
    if (segno_offset) segno_offset_ = 1; else segno_offset_ = 0;
    char char_kp[5];        snprintf(char_kp, sizeof(char_kp), "%d", kp_);                          //SD.t_kp = kp;
    char char_ki[5];        snprintf(char_ki, sizeof(char_ki), "%d", ki_);                          //SD.t_ki = ki;
    //printf("ki scritto in sd --> %d\n", ki_);
    char char_kd[5];        snprintf(char_kd, sizeof(char_kd), "%d", kd_);                          //SD.t_kd = kd;
    char char_offset[5];    snprintf(char_offset, sizeof(char_offset), "%d", offset_);              //SD.t_offset = offset;
    char char_segno_offset[5];  snprintf(char_segno_offset, sizeof(char_segno_offset), "%d", segno_offset_);

    char string[100];
    strcpy(string, char_v_min); strcat(string, ";"); strcat(string, char_v_max); strcat(string, ";"); strcat(string, char_v_rev); strcat(string, ";"); strcat(string, char_v_tar); strcat(string, ";"); strcat(string, char_v_imp); strcat(string, ";"); strcat(string, char_c_max);
    strcat(string, ";"); strcat(string, char_t_min); strcat(string, ";"); strcat(string, char_t_max); strcat(string, ";"); strcat(string, char_t_imp);
    strcat(string, ";"); strcat(string, char_kp); strcat(string, ";"); strcat(string, char_ki); strcat(string, ";"); strcat(string, char_kd); strcat(string, ";"); strcat(string, char_offset); strcat(string, ";"); strcat(string, char_segno_offset); strcat(string, ";");

    ESP_LOGI("Stringa in SD --> ", "%s", string);

    /*if(!SD.busy) { 
        SD_write(&SD, 4, string); 
        FAILED_write_valori_in_SD = 0; 
        valori_cambiati = 0;
    } else { 
        ESP_LOGE(TAG_sd, "SD in uso, non sono riuscito a scrivere il file"); 
        FAILED_write_valori_in_SD = 1; 
    }*/

    if (SD_operation(&SD, 1, 4, string) == ESP_OK) {
        FAILED_write_valori_in_SD = 0; 
        valori_cambiati = 0;
    } else {
        ESP_LOGE(TAG_sd, "ERRORE SD -> non sono riuscito a scrivere il file valori"); 
        FAILED_write_valori_in_SD = 1; 
    }
}


void MSG_read_valori_in_SD() {
    /*if(!SD.busy) {
        SD_read(&SD, 4);
        FAILED_read_valori_in_SD = 0;
    } else {
        ESP_LOGE(TAG_sd, "SD in uso, non sono riuscito a leggere il file");    //SD_buffer_readed  //  --> riga di valori --> da dividere
        FAILED_read_valori_in_SD = 1;
        while (FAILED_read_valori_in_SD) {
            if(!SD.busy) {
                SD_read(&SD, 4);
                FAILED_read_valori_in_SD = 0;
            }
            delay(10);
        }
    }*/

    if (SD_operation(&SD, 0, 4, NULL) == ESP_OK) FAILED_read_valori_in_SD = 0; else FAILED_read_valori_in_SD = 1;

    int valori[14];
    int cont = 0;
    char* token = strtok((char*)SD_buffer_readed, ";");
    while (token && (cont <= 13)) {
        //printf("cont = %d\n", cont);

        char* endPtr;
        long longValue = strtol(token, &endPtr, 10); // 10 indicates base 10
        if (*endPtr == '\0') {
            valori[cont] = (int)longValue; // Optional cast to int
            if (valori[cont] > 256) { errore_valori_salvati = 1; ESP_LOGE("ERRORE VALORI LETTI DA SD", "[%d] %d  -->  valore superiore a 256\n", cont, valori[cont]); }
            //printf("intValue = %d\n", valori[cont]);
            // Now, intValue contains the integer value
        }
        //printf("Token: %s\n\n", token);
        token = strtok(NULL, ";");

        cont ++;        
    }

    if (errore_valori_salvati == 0) {
        //  VALORI MOTORE
        velocita_minima = valori[0];        ESP_LOGI("VALORI LETTI DA SD", "v_min --> %d", velocita_minima);
        velocita_massima = valori[1];       ESP_LOGI("VALORI LETTI DA SD", "v_max --> %d", velocita_massima);
        velocita_reverse = valori[2];       ESP_LOGI("VALORI LETTI DA SD", "v_rev --> %d", velocita_reverse);
        velocita_tartaruga = valori[3];     ESP_LOGI("VALORI LETTI DA SD", "v_tar --> %d", velocita_tartaruga);
        velocita_impostata = valori[4];     ESP_LOGI("VALORI LETTI DA SD", "v_imp --> %d", velocita_impostata);
        corrente_massima = valori[5];       ESP_LOGI("VALORI LETTI DA SD", "c_max --> %d", corrente_massima);
        //  VALORI PID 1
        temperatura_minima = valori[6];     ESP_LOGI("VALORI LETTI DA SD", "t_min --> %d", temperatura_minima);
        temperatura_massima = valori[7];    ESP_LOGI("VALORI LETTI DA SD", "t_max --> %d", temperatura_massima);
        target_temperature = valori[8];     ESP_LOGI("VALORI LETTI DA SD", "t_imp --> %d", target_temperature);
        //  VALORI PID 2
        kp = valori[9]/10.0;                  ESP_LOGI("VALORI LETTI DA SD", "kp    --> %f", kp);
        ki = valori[10]/100.0;                 ESP_LOGI("VALORI LETTI DA SD", "ki    --> %f", ki);         //    printf("ki    --> %f ____ %d\n", ki, valori[10]);
        kd = valori[11]/10.0;                 ESP_LOGI("VALORI LETTI DA SD", "kd    --> %f", kd);
        offset = valori[12]/*/10.0*/;             ESP_LOGI("VALORI LETTI DA SD", "offset--> %f", offset);
        if ((valori[13]) == 1) segno_offset = 1; else segno_offset = 0;      ESP_LOGI("VALORI LETTI DA SD", "segno_offset--> %d\n", (valori[13]));
    } else {
        ESP_LOGE(TAG_sd, "ERRORE valori letti da SD --> tengo valori di default");
        MSG_valori_in_SD();
    }

}   //  SE DEVO INVIARE VALORI, CONTROLLO SE VALORI(IN VARIABILI) E VALORI (IN FILE SD) SONO DIVERSI O CON UN FLAG SEGNALO QUANDO E' STATO MODIFICATO ALMENO UNO DEI VALORI
    //  -->     DOVREBBERO ESSERE SEMPRE SINCRONIZZATI (perchè vengono modificate le variabili e poi salvate in sd se modificate, quando mi riaccendo leggo in sd e salvo in variabili)


void MSG_update_value_in_SD() {
    if (valori_cambiati || FAILED_write_valori_in_SD) {
        if ( ((millis() - t_valori_cambiati) > 3000/*10000*/) && ((millis() - t_temperatura_sd) > 1000) ) {
            MSG_valori_in_SD();
            t_valori_cambiati = millis();
        }
    }
}


//  *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*


void SD_update_statistics() {
    bool update_statistics = 0;
    while (!update_statistics) {
        if ( (FAILED_write_statistiche_in_SD && ((millis() - t_statistiche_in_SD) > 1000)) || ((millis() - t_statistiche_in_SD) > 1000) ) {

            /*
            //  MACCHINA
            millis_macchina += millis() - inizio_macchina;
            inizio_macchina = millis();
            minuti_macchina += (uint32_t)roundf( millis_macchina / 60000 );
            //  PID
            if (PID_getState(&PID) == 1) {
                if (inizio_pid != 0) millis_pid += millis() - inizio_pid;
                inizio_pid = millis();
            }
            minuti_pid += (uint32_t)roundf( millis_pid / 60000 );
            //  MOTORE
            if (stato_motore != 0) {
                if (inizio_motore != 0) millis_motore += millis() - inizio_motore;
                inizio_motore = millis();
            }
            minuti_motore += (uint32_t)roundf( millis_motore / 60000 );
            */

            char char_minuti_macchina[10];      snprintf(char_minuti_macchina, sizeof(char_minuti_macchina), "%ld", minuti_macchina);
            char char_minuti_pid[10];           snprintf(char_minuti_pid, sizeof(char_minuti_pid), "%ld", minuti_pid);
            char char_minuti_motore[10];        snprintf(char_minuti_motore, sizeof(char_minuti_motore), "%ld", minuti_motore);
            char string[50];
            strcpy(string, char_minuti_macchina); strcat(string, ";"); strcat(string, char_minuti_pid); strcat(string, ";"); strcat(string, char_minuti_motore);

            /*if(!SD.busy) { 
                if ( SD_write(&SD, 2, string) == ESP_OK) {
                    FAILED_write_statistiche_in_SD = 0;
                    update_statistics = 1;
                    ESP_LOGI(TAG_sd, "Minuti di utilizzo scritti in SD nel file statistics.txt");
                } else {
                    FAILED_write_statistiche_in_SD = 1;
                    ESP_LOGE(TAG_sd, "ERRORE --> non sono riuscito a scrivere il file"); 
                }
            } else { 
                ESP_LOGE(TAG_sd, "SD in uso, non sono riuscito a scrivere il file"); 
                FAILED_write_statistiche_in_SD = 1; 
            }*/

            if (SD_operation(&SD, 1, 2, string) == ESP_OK) {
                FAILED_write_statistiche_in_SD = 0;
                update_statistics = 1;
                ESP_LOGI(TAG_sd, "Minuti di utilizzo scritti in SD nel file statistics.txt");
            } else {
                FAILED_write_statistiche_in_SD = 1;
                ESP_LOGE(TAG_sd, "ERRORE --> non sono riuscito a scrivere il file"); 
            }


            t_statistiche_in_SD = millis();
        }
    }
}


void SPIFF_update_statistics(ConfigData* config) {
    if ((millis() - t_statistiche_in_SPIFF) > 60000) {
        if (config->busy == 0) {
            //  MACCHINA
            millis_macchina += millis() - inizio_macchina;
            inizio_macchina = millis();
            //  PID
            if (PID_getState(&PID) == 1) {
                if (inizio_pid != 0) millis_pid += millis() - inizio_pid;
                inizio_pid = millis();
            }
            //  MOTORE
            if (stato_motore != 0) {
                if (inizio_motore != 0) millis_motore += millis() - inizio_motore;
                inizio_motore = millis();
            }

            /*if (SPIFFS_write_(&SPIFFS) == ESP_OK) ESP_LOGI("writing millis.csv", "file millis.csv in spiff scritto");
            else ESP_LOGE("writing millis.csv", "ERRORE --> non ho scritto il file millis.csv");*/

            if (SPIFFS_write_(&SPIFFS) != ESP_OK) ESP_LOGE("writing millis.csv", "ERRORE --> non ho scritto il file millis.csv");

            t_statistiche_in_SPIFF = millis();
        } else {
            ESP_LOGE("SPIFF_update_statistics", "SPIFF busy in un'altra operazione, riprovo tra altri 10 secondi");
            t_statistiche_in_SPIFF = millis();
        }
    }    
}


void millisToOreMsbLsb() {    
    //  MACCHINA
    millis_macchina += millis() - inizio_macchina;                      ESP_LOGI("INVIO ORE", "millis_macchina: %lu", millis_macchina);
    inizio_macchina = millis();
    minuti_macchina = (uint32_t)roundf(millis_macchina / 60000);        ESP_LOGI("INVIO ORE", "minuti_macchina: %lu", minuti_macchina);
    ore_macchina = (uint16_t)roundf(minuti_macchina / 60);              ESP_LOGI("INVIO ORE", "ore_macchina: %u", ore_macchina);
    ore_utilizzo_mac_MSB = ((ore_macchina >> 8) & 0xff);                ESP_LOGI("INVIO ORE", "ore_utilizzo_mac_MSB: %u", ore_utilizzo_mac_MSB);
    ore_utilizzo_mac_LSB = (ore_macchina & 0xff);                       ESP_LOGI("INVIO ORE", "ore_utilizzo_mac_LSB: %u\n", ore_utilizzo_mac_LSB);

    //  PID
    if (PID_getState(&PID) == 1) {
        if (inizio_pid != 0) { millis_pid += millis() - inizio_pid;       ESP_LOGI("INVIO ORE", "millis_pid: %lu", millis_pid); }
        inizio_pid = millis();
    } else ESP_LOGI("INVIO ORE", "millis_pid: %lu", millis_pid);
    minuti_pid = (uint32_t)roundf(millis_pid / 60000);                  ESP_LOGI("INVIO ORE", "minuti_pid: %lu", minuti_pid);
    ore_pid = (uint16_t)roundf(minuti_pid / 60);                        ESP_LOGI("INVIO ORE", "ore_pid: %u", ore_pid);
    ore_pid_MSB = ((ore_pid >> 8) & 0xff);                              ESP_LOGI("INVIO ORE", "ore_pid_MSB: %u", ore_pid_MSB);
    ore_pid_LSB = (ore_pid & 0xff);                                     ESP_LOGI("INVIO ORE", "ore_pid_LSB: %u\n", ore_pid_LSB);

    //  MOTORE
    if (stato_motore != 0) {
        if (inizio_motore != 0) { millis_motore += millis() - inizio_motore;  ESP_LOGI("INVIO ORE", "millis_motore: %lu", millis_motore); }
        inizio_motore = millis();
    } else ESP_LOGI("INVIO ORE", "millis_motore: %lu", millis_motore);
    minuti_motore = (uint32_t)roundf(millis_motore / 60000);              ESP_LOGI("INVIO ORE", "minuti_motore: %lu", minuti_motore);
    ore_motore = (uint16_t)roundf(minuti_motore / 60);                    ESP_LOGI("INVIO ORE", "ore_motore: %u", ore_motore);
    ore_mot_MSB = ((ore_motore >> 8) & 0xff);                           ESP_LOGI("INVIO ORE", "ore_mot_MSB: %u", ore_mot_MSB);
    ore_mot_LSB = (ore_motore & 0xff);                                  ESP_LOGI("INVIO ORE", "ore_mot_LSB: %u\n", ore_mot_LSB);
}
