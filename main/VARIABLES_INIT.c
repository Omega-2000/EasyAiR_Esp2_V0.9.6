//#include "TWAI.h"
//#include "PID.h"
//#include "CONFIG_FILE.h"
//#include "SD.h"
//#include "EMERGENZE.h"
//#include "PEDALE.h"
//#include "ELETTROVALVOLE.h"
#include "MSG_LISTENER.h"
#include "TWAI_SELF_TEST.c"

#define INIT_TAG    "_INIT_"

//  TASK
TaskHandle_t globalTaskHandler = NULL;
TaskHandle_t globalTaskHandler_2 = NULL;
TaskHandle_t globalTaskHandler_3 = NULL;

//  TWAI
twai_message_t message;
uint32_t idx;
uint8_t buf[LEN];
uint8_t old_can_buf[LEN];
uint8_t priorita = -1;          
uint8_t mittente = -1;
uint8_t destinatario = -1;

//  PING
uint16_t versioneFW = 101;
uint16_t versioneFW_esp1 = 0;
uint16_t versioneFW_esp3 = 0;
bool pingReply_esp1 = 0;
bool pingReply_esp3 = 0;
bool pingOK = 0;
bool pingReply_received = 0;
bool pingRequest_esp1 = 0;
bool pingRequest_esp3 = 0;
bool pingRequest_received = 0;
unsigned long t_pingRequest = 0;

//  VALORI INIZIALI
bool richiesta_esp1 = 0;
bool richiesta_esp3 = 0;
bool valori_motore = 0;
bool valori_pid_1 = 0;
bool valori_pid_2 = 0;
uint8_t kp_;
uint8_t ki_;
uint8_t kd_;
uint8_t offset_;
uint8_t segno_offset_;

//  STATO MACCHINA AL RIAVVIO DI UN ESP
uint8_t motore_;
uint8_t modalita_;
uint8_t pedale_;
uint8_t emergenza_;
uint8_t pid_;
uint8_t stato_s1_;
uint8_t funzione_s1_;

//  PID
Pid PID;
float temperature;
float temperature_offset;
unsigned long t_temperatura_attuale = 0;
bool allow_pid = 0;
uint8_t pid_restart = 0;
//uint8_t stato_pid = 0;
uint8_t last_stato_pid = 0;
uint8_t temperatura_attuale = 0;
uint8_t last_temperatura_attuale = 0;
uint8_t target_temperature = 0;             //  SD / spiff
uint8_t temperatura_minima = 20;            //  SD / spiff
uint8_t temperatura_massima = 60;           //  SD / spiff
float kp = 15.0;                            //  SD / spiff
float ki = 0.0;                             //  SD / spiff
float kd = 0.0;                             //  SD / spiff
float offset = 0.0;                         //  SD / spiff
bool segno_offset = 0;                      //  SD / spiff          //  0x00 --> +  ,  0x01 --> -
bool watchdog = 0;                                               //  0 --> off  ,  1 --> on
unsigned long t_watchdog = 0;
uint8_t init_or_deinit = 0;
unsigned long t_pidon = 0;
unsigned long t_pidoff = 0;
bool watchdog_init = 0;
uint8_t cont_prima_temp_errata = 0;
uint8_t cont_temp_errata = 0;
bool PID_start_riavvio = 0;
unsigned long last_pid = 0;
bool PID_1_st_ch = 0;
float last_temperature = 0.0;
uint8_t display_temperature = 0;
uint8_t new_temperature = 0;
int cont_diff_1 = 0;
int cont_diff_2 = 0;
int cont_target_diff_1 = 0;
int cont_display_0 = 0;
bool ped_pre_al_pass_mod_ped = 0;
unsigned long last_send_temperature = 0;
float integrale_ = 0.0;
bool accept_temperature = 0;
bool prima_rampa_fatta = 0;

//  MOTORE
uint8_t stato_motore = 0;
uint8_t last_stato_motore = 0;
uint8_t velocita_impostata = 0;             //  SD / spiff
uint8_t velocita_minima = 0;                //  SD / spiff
uint8_t velocita_massima = 20;              //  SD / spiff
uint8_t velocita_reverse = 10;              //  SD / spiff
uint8_t velocita_tartaruga = 3;             //  SD / spiff
uint8_t corrente_massima = 10;              //  SD / spiff          //  (A)

//  EMERGENZE
bool emergenze = 0;
bool emergenze_funghi = 0;
bool emergenze_fotocellule = 0;
bool allow_photocell = 0;
bool fotocellula_interrotta = 0;
bool fungo_premuto = 0;
bool reset_premuto = 0;
unsigned long t_reset_premuto = 0;
bool az_reset = 0;
unsigned long t_start_az_1_int = 0;
unsigned long t_stop_az_1_int = 0;
unsigned long t_pin_em = 0;
int8_t EM_azione = -1;
int8_t EM_azione_1 = -1;
int8_t EM_azione_2 = -1;
unsigned long t_reset_rilasciato = 0;
unsigned long t_fotocellula_rilasciata = 0;
unsigned long t_rilasciati = 0;
bool primo_reset = 0;
bool fotoc_blocc_abbassa = 0;
uint8_t n_emergenza = 0;    //  0 nessuna emergenza , 1 fotocellula , 2 fungo sx , 3 fungo dx
bool pid_off_em = 0;
bool em_funghi = 0;
int print = 0;
int cont_print = 0;
bool force_start_az_0 = 0;
unsigned long t_print = 0;

//  ELETTROVALVOLE
#define ALZA        37                      //  OUT1
#define ABBASSA     36                      //  OUT2
bool allow_abbassa = 0;
bool abbassa = 0;
bool alza = 0;
unsigned long t_abbassa = 0;
unsigned long t_alza = 0;
unsigned long t_msg_abbassa = 0;
unsigned long t_msg_alza = 0;
unsigned long t_abbassa_off = 0;
unsigned long t_alza_off = 0;

//  PEDALE
#define PEDALE      15
bool stato_pedale = 0;
bool allow_pedale = 0;
bool pedale_premuto = 0;
bool pedale = 0;
bool stato_tartaruga = 0;                   //  TARTARUGA
bool stato_s1 = 0;                          //  0 attivo  -  1 non attivo (da messaggio stato macchina) --> sennò è 1 attivo e 0 non attivo
bool funzione_s1 = 0;                       //  0 S.P.  -  1 Relè
bool funzione_premuto = 1;
bool funzione_rilasciato = 0;
bool da_man_a_ped = 0;
bool fotoc_dis_da_reset = 0;

//  SPIFFS CONFIG_FILE
ConfigData SPIFFS;
//  ORE DI UTILIZZO
unsigned long inizio_macchina = 0;
unsigned long inizio_pid = 0;
unsigned long inizio_motore = 0;
uint32_t minuti_macchina = 0;
uint32_t minuti_pid = 0;
uint32_t minuti_motore = 0;
unsigned long millis_macchina = 0;          //3600000;
unsigned long millis_pid = 0;               //3600000;
unsigned long millis_motore = 0;            //3600000;
bool FAILED_write_statistiche_in_SD = 0;
uint16_t ore_macchina = 0;
uint16_t ore_pid = 0;
uint16_t ore_motore = 0;
uint8_t ore_pid_MSB = 0;
uint8_t ore_pid_LSB = 0;
uint8_t ore_mot_MSB = 0;
uint8_t ore_mot_LSB = 0;
uint8_t ore_utilizzo_mac_MSB = 0;
uint8_t ore_utilizzo_mac_LSB = 0;
unsigned long t_statistiche_in_SD = 0;
unsigned long t_statistiche_in_SPIFF = 0;
bool ready = 0;
bool lettura_ore_in_SPIFF = 0;
bool crea_file_in_SPIFF;
uint8_t risposta;

//  SD
Sd SD;
unsigned long t_temperatura_sd = 0;
int somma_temperatura_attuale = 0;
int cont_somma_temperatura_attuale = 0;
int media_temperatura_attuale = 0;
char SD_buffer_readed[256];
bool valori_cambiati = 0;
unsigned long t_valori_cambiati = 0;
bool FAILED_write_valori_in_SD = 0;
bool FAILED_read_valori_in_SD = 0;
unsigned long t_sd_closed = 0;
bool errore_valori_salvati = 0;
unsigned long t_minuti = 0;

//  ERRORI
bool I2c_works = 0;
bool Spiff_works = 0;
bool SD_works = 0;
bool Fotocellula_works = 0;

//  STEP E CONDIZIONI NEL GIRO DEL PROGRAMMA
bool mi_sono_riavviato = 0;
bool macchina_pronta = 0;
bool update_finished = 0;
unsigned long t_restart_avvio = 0;

//  UPDATE
bool perform_update = 0;
//int myFirmwareVersion = 101;

//  FUNZIONI
void taskMsg(void *pvParameters);
void taskPid(void *pvParameters);
void t_init();

void t_init() {     //  fai partire i timer effettivamente quando la macchina è pronta
    t_temperatura_attuale = millis();
    t_temperatura_sd = millis();
    t_abbassa = millis();
    t_alza = millis();
    t_valori_cambiati = millis();
    t_statistiche_in_SD = millis();
    t_statistiche_in_SPIFF = millis();
    inizio_macchina = millis();
    t_sd_closed = millis();
}
