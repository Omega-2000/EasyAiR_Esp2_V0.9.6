#ifndef __PEDALE_H__
#define __PEDALE_H__


#define PEDALE_PIN      15      //  OUT1
#define RELE_PIN         9
#define ABBASSA         36      //  OUT2

#define delay(ms)   vTaskDelay(ms/portTICK_PERIOD_MS)


#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <driver/gpio.h>
#include "esp_timer.h"                      //  micros      esp_timer_get_time() / 1000    =   millis
#include "esp_log.h"
#include "TWAI.h"
#include "EMERGENZE.h"
#include "ELETTROVALVOLE.h"

extern bool pedale_premuto;
static bool ultimo_pedale_premuto = 0;
extern bool stato_pedale;                   //  0 -> manuale  ,  1 -> pedale
extern bool stato_tartaruga;                //  0 -> tartaruga OFF  ,  1 -> tartaruga ON
extern bool stato_s1;                       //  0 attivo  -  1 non attivo
extern bool funzione_s1;                    //  0 S.P.  -  1 Relè
extern bool ped_pre_al_pass_mod_ped;
extern bool funzione_premuto;
extern bool funzione_rilasciato;
extern bool da_man_a_ped;
extern bool fotoc_dis_da_reset;

void PEDALE_init();
void PEDALE_loop();


#ifdef __cplusplus
}
#endif

#endif  // __PEDALE_H__
