#ifndef __MSG_LISTENER_H__
#define __MSG_LISTENER_H__


#define PING_TAG    "_PING_"
#define MSG_TAG     "_MSG_"


#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <driver/gpio.h>
#include "esp_log.h"
#include <string.h>

#include "PID.h"
#include "TWAI.h"
#include "EMERGENZE.h"
#include "PEDALE.h"
//#include "ELETTROVALVOLE.h"
#include "CONFIG_FILE.h"
#include "SD.h"
#include "Updater.h"

//  MAIN
extern TaskHandle_t globalTaskHandler;
extern TaskHandle_t globalTaskHandler_2;
extern bool macchina_pronta;
extern unsigned long t_restart_avvio;

//  TWAI
extern uint8_t priorita;
extern uint8_t mittente;
extern uint8_t destinatario;

//  PING
extern unsigned long t_pingRequest;
extern bool pingReply_esp1;
extern bool pingReply_esp3;
extern bool pingReply_received;
extern bool pingRequest_esp1;
extern bool pingRequest_esp3;
extern bool pingRequest_received;
extern bool pingOK;

//  VALORI INIZIALI
extern bool richiesta_esp1;
extern bool richiesta_esp3;
extern bool valori_motore;
extern bool valori_pid_1;
extern bool valori_pid_2;
extern uint8_t kp_;
extern uint8_t ki_;
extern uint8_t kd_;
extern uint8_t offset_;
extern uint8_t segno_offset_;

//  STATO MACCHINA AL RIAVVIO DI UN ESP
extern uint8_t motore_;
extern uint8_t modalita_;
extern uint8_t pedale_;
extern uint8_t emergenza_;
extern uint8_t pid_;
extern uint8_t stato_s1_;
extern uint8_t funzione_s1_;

//  PID
extern Pid PID;
extern bool PID_1_st_ch;

//  MOTORE
extern uint8_t velocita_minima;
extern uint8_t velocita_massima;
extern uint8_t velocita_reverse;
extern uint8_t velocita_tartaruga;
extern uint8_t corrente_massima;
extern uint8_t velocita_impostata;

//  ORE DI UTILIZZO
extern uint16_t ore_macchina;
extern uint16_t ore_pid;
extern uint16_t ore_motore;
extern uint8_t ore_pid_MSB;
extern uint8_t ore_pid_LSB;
extern uint8_t ore_mot_MSB;
extern uint8_t ore_mot_LSB;
extern uint8_t ore_utilizzo_mac_MSB;
extern uint8_t ore_utilizzo_mac_LSB;
extern uint8_t risposta;
extern bool ready;

//  SD
extern bool valori_cambiati;
extern bool FAILED_write_valori_in_SD;
extern bool FAILED_read_valori_in_SD;
extern unsigned long t_sd_closed;
extern bool errore_valori_salvati;

//  UPDATE
extern bool perform_update;

//  MESSAGE LISTENER
void macchina_ready();
void MSG_message_receiver();

//  SD OPERATIONS
void MSG_valori_in_SD();
void MSG_read_valori_in_SD();
void MSG_update_value_in_SD();
//  ore di utilizzo
void SD_update_statistics();
void SD_read_statistics();

//  SPIFF OPERATIONS
void MSG_lettura_in_SPIFF();
void SPIFF_update_statistics(ConfigData* config);

void millisToOreMsbLsb();


#ifdef __cplusplus
}
#endif

#endif  // __MSG_LISTENER_H__
