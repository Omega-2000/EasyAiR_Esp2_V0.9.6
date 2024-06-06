#ifndef __EMERGENZE_H__
#define __EMERGENZE_H__


/*
    PER ABILITARE ANCHE LA FOTOCELLULA N.RO 2   --> "ALZARE" ENABLE ANCHE IN REVERSE (già pronto in message_receiver() in MSG_LISTENER.c)
                                                --> INIZIALIZZARE GPIO FOTOC2 E INSTALLARE L INTERRUPT (già pronto in EM_configure_gpio_interrupt() in EMERGENZE.c)
                                                --> CREARE LOGICA NEI VARI COMPONENTI CON INTEGRAZIONE DELLA SECONDA FOTOCELLULA
*/


#define EMERGENZE_TAG   "_EMERGENZE_"
#define FOTOC_TAG       "_FOTOCELLULA_"

#define RESET_EM    48
#define FOTOC1       6
#define FOTOC2       7
#define EM1         34
#define EM2         35
#define EMERGENZE   38
#define PEDALE_PIN  15
#define TEST_FOTO   33
#define ENABLE       8

#define PHOTOCELL_DAVANTI   0x00
#define PHOTOCELL_RETRO     0x01
#define FUNGO_DX            0x02
#define FUNGO_SX            0x03
#define SERIE_EMERGENZE     0x04
#define LONG_RESET          0x05


#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "esp_log.h"
#include <C:\Users\alessandro\esp\esp-idf\components\esp_rom\include\esp32s3\rom\gpio.h>
#include "TWAI.h"
#include "PID.h"
#include "ELETTROVALVOLE.h"


//  allow
extern bool allow_photocell;
extern bool allow_pid;
extern bool allow_abbassa;
extern bool allow_pedale;

extern bool mi_sono_riavviato;
extern uint8_t stato_motore;
extern uint8_t last_stato_motore;
extern uint8_t last_stato_pid;
extern uint8_t pid_restart;
extern bool Fotocellula_works;
extern bool emergenze;
extern bool emergenze_funghi;
extern bool emergenze_fotocellule;
extern bool reset_premuto;
extern bool fotocellula_interrotta;
extern bool fungo_premuto;
extern unsigned long t_reset_premuto;
extern bool az_reset;
extern unsigned long t_start_az_1_int;
extern unsigned long t_stop_az_1_int;
extern int8_t EM_azione;
extern int8_t EM_azione_1;
extern int8_t EM_azione_2;
extern unsigned long t_pin_em;
extern bool pedale;
extern unsigned long t_reset_rilasciato;
extern unsigned long t_fotocellula_rilasciata;
extern unsigned long t_rilasciati;
extern bool primo_reset;
extern uint8_t n_emergenza;

extern bool fotoc_blocc_abbassa;

extern bool pid_off_em;

extern bool em_funghi;

extern bool fotoc_dis_da_reset;

extern int print;
extern int cont_print;

extern bool force_start_az_0;

void IRAM_ATTR EM_emergency(bool funghi);
void IRAM_ATTR EM_fotocellula1(void* arg);
void IRAM_ATTR EM_fungo1(void* arg);
void IRAM_ATTR EM_fungo2(void* arg);
void IRAM_ATTR EM_reset(void* arg);
void IRAM_ATTR EM_pedale(void* arg);
void IRAM_ATTR EM_emergenze(void* arg);
void EM_configure_gpio_interrupt();
bool EM_test_fotocellule();
//void EM_reset_loop();

void EM_start_azione(int8_t azione);
void EM_start_azione_interrupt(int8_t azione);
void EM_loop();

//void EM_check_start();


#ifdef __cplusplus
}
#endif

#endif  // __EMERGENZE_H__
