# PID

**DIPENDENZE**
- "TWAI.h" --> componente utilizzato per la gestione dei messaggi canbus da inviare e da ricevere
- "SD.h" --> componente utilizzato per la gestione delle operazioni all'interno dell'sd
- "driver/i2c.h" --> componente utilizzato per inizializzare ed utilizzare l'i2c
- "esp_task_wdt.h" --> componente utilizzato per la gestione del watchdog

**FUNZIONAMENTO**
Per poter utilizzare tutte le funzioni riguardanti il pid, all'inizio del programma serve inizializzare l'i2c, il pid e impostare i valori che gli servono per funzionare. Successivamente si potrà utilizzare a pieno il funzionamento del pid. Servirà far partire il pid con la funzione "PID_start()" e si potrà richiamare in loop la funzione "PID_task()" per fargli controllare la temperatura letta dal sensore e successivamente richiamerà la funzione "PID_control_heater()" che a sua volta richiamerà "PID_calculateValue()" e accenderà/spegnerà l'ssr collegato all'uscita del pid. Per fermare il funzionamento del pid basterà chiamare la funzione "PID_stop()".

**CODICE**
- PID_i2c_master_init();  //  inizializza i2c
- PID_read_temperature();  //  ritorna la temperatura letta dal sensore
- PID_temperatura_attuale_loop(Pid* pid);  //  invia la temperatura letta al microcontrollore del display e la salva all'interno dell'sd
- PID_control_heater(Pid* pid, float c_temp);  //  accende e spegne l'ssr a seconda del funzionamento del pid
- PID_task(Pid* pid);  //  controlla la temperatura letta dal sensore e se accettabile richiama la funzione "PID_control_heater", altrimenti manda l'errore e lo salva in sd
- PID_begin(Pid* pid, uint8_t SSR_pin, float kp_, float ki_, float kd_);  //  inizializza il pid
- PID_setPidParameters(Pid* pid, float kp_, float ki_, float kd_);  //  imposta i parametri passati come coefficenti del pid
- PID_setOffset(Pid* pid, float offset_);  //  imposta il parametro passato come offset del pid
- PID_setOutPeriod(Pid* pid, uint32_t period);  //  imposta il paratro passato come periodo del funzionamento del pid
- PID_setTemp(Pid* pid, uint8_t t_set);  //  imposta il parametro passato come target di temperatura che il pid deve raggiungere
- PID_start(Pid* pid, uint8_t t_set);  //  fa partire il funzionamento del pid
- PID_stop(Pid* pid);  //  ferma il funzionamento del pid
- PID_getSetTemp(Pid* pid);  //  ritorna il valore del target della temperatura da raggiugere
- PID_getState(Pid* pid);  //  ritorna lo stato del funzionamento del pid
- PID_getKp(Pid* pid);  //  ritorna il coefficente proporzionale
- PID_getKi(Pid* pid);  //  ritorna il coefficente integrato
- PID_getKd(Pid* pid);  //  ritorna il coefficente derivato
- PID_getOffset(Pid* pid);  //  ritorna l'offset
- PID_calculateValue(Pid* pid);  //  calcola i tre valori per il funzionamento del pid
- PID_temp_in_SD();  //  scrive all'interno dell'sd una stringa con le statistiche attuali del pid
- PID_check_temperatura_attuale(Pid* pid, uint8_t t_attuale);  //  controlla la temperatura da inviare al microcontrollore del display
- PID_init_WDT();  //  inizializza il watchdog
- PID_deinit_WDT();  //  deinizializza il watchdog
- PID_avvio_in_SD();  //  scrive "AVVIO MACCHINA" all'avvio della macchina all'interno del file "temperatures.csv" in sd
