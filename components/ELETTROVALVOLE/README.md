# ELETTROVALVOLE

**DIPENDENZE**
#include <stdio.h>
#include <driver/gpio.h>
#include "esp_timer.h"
#include "esp_log.h"

**FUNZIONAMENTO**
Richiamare la funzione "ELETTROVALVOLE_init()" per inizializzare i 2 gpio utilizzati da questo componente e la funzione "ELETTROVALVOLE_off()" all'interno del loop principale per fermare l'esecuzione delle funzioni "alza" e "abbassa" quando vengono eseguite.

**CODICE**
- ELETTROVALVOLE_init();    //  inizializza gpio "alza" e "abbassa"
- ELETTROVALVOLE_off();     //  controlla e ferma l'esecuzione delle funzioni "alza" e "abbassa"
