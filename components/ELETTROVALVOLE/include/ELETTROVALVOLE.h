#ifndef __ELETTROVALVOLE_H__
#define __ELETTROVALVOLE_H__


#define ELETTROVALVOLE_TAG  "_ELETTROVALVOLE_"
#define ALZA                37  //  OUT1
#define ABBASSA             36  //  OUT2

#define SERV1               9   //  SERV1 --> RELE' DA POTER IMPOSTARE SU S1


#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <driver/gpio.h>
#include "esp_timer.h"      //  micros      esp_timer_get_time() / 1000    =   millis
#include "esp_log.h"

//extern bool allow_abbassa;
extern bool abbassa;
extern bool alza;
extern unsigned long t_abbassa;
extern unsigned long t_alza;
extern unsigned long t_msg_abbassa;
extern unsigned long t_msg_alza;
extern unsigned long t_abbassa_off;
extern unsigned long t_alza_off;

void ELETTROVALVOLE_init();
void ELETTROVALVOLE_off();


#ifdef __cplusplus
}
#endif

#endif  // __ELETTROVALVOLE_H__
