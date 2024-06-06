#ifndef __CONFIG_FILE_H__
#define __CONFIG_FILE_H__


#define SPIFF_TAG    "_SPIFFS_"
#define CONFIG_FILE_PATH "/spiffs/data.json"
#define CONFIG_FILE_SIZE sizeof(ConfigData)


#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "PID.h"
/*#include "cJSON.h"
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>*/

// Structure of the data that need to be retrived on startup, they are saved  in data.json spiff file
typedef struct {
    bool busy;

    unsigned long millis_macchina;
    unsigned long millis_pid;
    unsigned long millis_motore;

} ConfigData;

extern ConfigData SPIFFS;

extern bool Spiff_works;

extern unsigned long inizio_macchina;
extern unsigned long inizio_pid;
extern unsigned long inizio_motore;
extern uint32_t minuti_macchina;
extern uint32_t minuti_pid;
extern uint32_t minuti_motore;
extern unsigned long millis_macchina;
extern unsigned long millis_pid;
extern unsigned long millis_motore;

extern unsigned long t_statistiche_in_SPIFF;
extern bool lettura_ore_in_SPIFF;
extern bool crea_file_in_SPIFF;


esp_err_t SPIFFS_init(ConfigData* config);
esp_err_t SPIFFS_write_(ConfigData* config);        //  ( SALVA SU FILE )
int8_t SPIFFS_read_(ConfigData* config);            //  ( SALVA SU OGGETTO )
bool SPIFFS_check_(ConfigData* config, uint8_t minT, uint8_t maxT, uint8_t kp, uint8_t ki, uint8_t kd, uint8_t offset, uint8_t minS, uint8_t maxS, uint8_t reverseS, uint8_t turtleS);      //  CHECK DA OGGETTO A VALORI PASSATI COME PARAMETRI
uint8_t SPIFFS_remove_(const char *filename);
//void SPIFFS_update_values(ConfigData* config, uint8_t minT, uint8_t maxT, uint8_t kp, uint8_t ki, uint8_t kd, uint8_t offset, uint8_t minS, uint8_t maxS, uint8_t reverseS, uint8_t turtleS);


#ifdef __cplusplus
}
#endif

#endif  // __CONFIG_FILE_H__
