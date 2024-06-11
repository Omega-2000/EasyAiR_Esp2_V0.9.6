#ifndef PID_H
#define PID_H


#define PID_TAG     "_PID_"
#define WDT_TIMEOUT  10000
#define HEATER_PIN GPIO_NUM_16          // GPIO pin number for the heater control
#define millis()    esp_timer_get_time() / 1000

#define I2C_MASTER_SCL_IO 21            // GPIO number for I2C master clock
#define I2C_MASTER_SDA_IO 26            // GPIO number for I2C master data
#define I2C_MASTER_NUM I2C_NUM_0        // I2C port number
#define I2C_MASTER_FREQ_HZ /*50000*/100000       // I2C master clock frequency
#define SENSOR_ADDR 0x5A                // I2C address of the MLX90614 sensor


#ifdef __cplusplus
extern "C" {
#endif


#include "TWAI.h"
#include "SD.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include <C:\Users\alessandro\esp\esp-idf\components\esp_rom\include\esp32s3\rom\gpio.h>
#include "C:\Users\alessandro\esp\esp-idf\components\esp_timer\include\esp_timer.h"  //  esp_timer_get_time() --> micros --> / 1000  --> millis
#include "esp_task_wdt.h"

typedef struct {
    uint8_t SSR_PIN;
    float curr_temp;
    float set_temp;
    float previous_error;
    float start_time;
    float elapsedTime;
    uint32_t Time, timePrev;
    bool state;
    uint32_t SSR_time, SSR_period/* = 2000*/;
    int PID_value;
    bool cooling;
    //PID constants
    float KP;       float KI;       float KD;
    float PID_p;    float PID_i;    float PID_d;
    float OFFSET;

} Pid;

extern Pid PID;
//extern uint8_t stato_pid;
extern uint8_t temperatura_attuale;
extern uint8_t last_temperatura_attuale;
extern bool I2c_works;
extern uint8_t target_temperature;  // temperatura impostata
extern uint8_t temperatura_minima;
extern uint8_t temperatura_massima;
extern float kp;
extern float ki;
extern float kd;
extern float offset;
extern bool segno_offset;
extern unsigned long t_temperatura_sd;
extern unsigned long t_temperatura_attuale;
extern int somma_temperatura_attuale;
extern int cont_somma_temperatura_attuale;
extern int media_temperatura_attuale;
extern unsigned long t_valori_cambiati;
extern float temperature;
extern float temperature_offset;
extern bool watchdog;
static esp_task_wdt_user_handle_t func_twdt_user_hdl;
extern unsigned long t_watchdog;
extern uint8_t init_or_deinit;
extern unsigned long t_pidon;
extern unsigned long t_pidoff;
extern bool watchdog_init;
extern unsigned long t_minuti;
extern uint8_t cont_prima_temp_errata;
extern uint8_t cont_temp_errata;
extern bool PID_start_riavvio;
extern float last_temperature;
extern uint8_t display_temperature;
extern uint8_t new_temperature;
extern int cont_diff_1;
extern int cont_diff_2;
extern int cont_target_diff_1;
extern int cont_display_0;
extern unsigned long last_send_temperature;
extern float integrale_;
extern bool accept_temperature;
extern bool prima_rampa_fatta;

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
//void PID_scrivi_file_SD(char *s);


#ifdef __cplusplus
}
#endif

#endif  // PID_H
