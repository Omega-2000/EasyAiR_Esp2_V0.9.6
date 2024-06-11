# PID

**DIPENDENZE**
- "TWAI.h" --> componente utilizzato per la gestione dei messaggi canbus da inviare e da ricevere
- "SD.h" --> componente utilizzato per la gestione delle operazioni all'interno dell'sd
- "driver/i2c.h" --> componente utilizzato per inizializzare ed utilizzare l'i2c
- "esp_task_wdt.h" --> componente utilizzato per la gestione del watchdog

**FUNZIONAMENTO**

**CODICE**
esp_err_t PID_i2c_master_init();
float PID_read_temperature();
void PID_temperatura_attuale_loop(Pid* pid);
void PID_control_heater(Pid* pid, float c_temp);
void PID_task(Pid* pid);
void PID_begin(Pid* pid, uint8_t SSR_pin, float kp_, float ki_, float kd_);
void PID_setPidParameters(Pid* pid, float kp_, float ki_, float kd_);
void PID_setOffset(Pid* pid, float offset_);
void PID_setOutPeriod(Pid* pid, uint32_t period);
void PID_setTemp(Pid* pid, uint8_t t_set);
void PID_start(Pid* pid, uint8_t t_set);
void PID_stop(Pid* pid);
uint8_t PID_getSetTemp(Pid* pid);
uint8_t PID_getState(Pid* pid);
float PID_getKp(Pid* pid);
float PID_getKi(Pid* pid);
float PID_getKd(Pid* pid);
float PID_getOffset(Pid* pid);
float PID_calculateValue(Pid* pid);

void PID_temp_in_SD();
void PID_check_temperatura_attuale(Pid* pid, uint8_t t_attuale);
//void PID_send_temperatura_iniziale(Pid* pid);

void PID_init_WDT();
void PID_deinit_WDT();
//void PID_check_WDT(Pid* pid);

void PID_avvio_in_SD();
