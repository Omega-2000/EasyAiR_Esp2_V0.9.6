#ifndef __TWAI_H__
#define __TWAI_H__


#define TWAI_TAG     "_TWAI_"


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/twai.h"
#include "TWAI_DEFINE.h"

//  VAR
extern twai_message_t message;                                                          //  usato per controllare la ricezione di byte uguali
extern uint32_t idx;                                                                    //  memorizza l'id dell'ultimo messaggio ricevuto
extern uint8_t buf[];                                                                   //  memorizza i byte di dati dell'ultimo messaggio ricevuto
extern uint8_t old_can_buf[];                                                           //  usato per controllare la ricezione di byte uguali

//  FUNZIONI BASE
esp_err_t TWAI_init(/*twai_config* config*/);                                           //  inizializza twai
esp_err_t TWAI_reinit();                                                                //  reinizializza twai
esp_err_t TWAI_send(uint32_t id, uint16_t flag, uint8_t len, uint8_t data[]);           //  invia messaggio
esp_err_t TWAI_send_interrupt(uint32_t id, uint16_t flag, uint8_t len, uint8_t data[]);
esp_err_t TWAI_get();                                                                   //  riceve messaggio
static void TWAI_handle_rx_message(twai_message_t message);                                  //  legge e memorizza messaggio

//  FUNZIONI INVIO COMANDI
void TWAI_pingRequest(uint8_t destinatario);
void TWAI_pingReply(uint16_t versioneFW, uint8_t destinatario);

void TWAI_sendStatoPid(uint8_t stato, uint8_t temperatura);
void TWAI_sendConfermaEsportaOreInSd();
void TWAI_sendOreUtilizzo(uint8_t risposta, uint8_t ore_riscaldamento_MSB, uint8_t ore_riscaldamento_LSB, uint8_t ore_motore_MSB, uint8_t ore_motore_LSB, uint8_t ore_utilizzo_macchina_MSB, uint8_t ore_utilizzo_macchina_LSB);
void TWAI_sendAzioneEmergenze(uint8_t stato, uint8_t azione);
void TWAI_sendAzioneEmergenze_interrupt(uint8_t stato, uint8_t azione);
void TWAI_sendStatoPedale(uint8_t stato, uint8_t azioni);
void TWAI_sendConfermaPedale();
void TWAI_sendConfermaTurtle();
void TWAI_sendConfermaS1();
void TWAI_sendConfermaElettrovalvole(uint8_t azione);
void TWAI_sendEmergenze(uint8_t fotocellula1, uint8_t fotocellula2, uint8_t fungo1, uint8_t fungo2, uint8_t pin_emergenze);
void TWAI_sendRichiestaValori(uint8_t tipo_valori);
void TWAI_sendValoriMotore(uint8_t destinatario, uint8_t v_min, uint8_t v_max, uint8_t v_reverse, uint8_t v_turtle, uint8_t v_impostata, uint8_t corrente_max);
void TWAI_sendValoriRiscaldamento1(uint8_t t_min, uint8_t t_max, uint8_t t_target);
void TWAI_sendValoriRiscaldamento2(uint8_t kp_, uint8_t ki_, uint8_t kd_, uint8_t offset_, uint8_t segno_offset_);
void TWAI_sendConfermaVelocita(uint8_t tipo_velocita, uint8_t valore);
void TWAI_sendMotorCommand(uint8_t comando);
void TWAI_sendError(uint8_t error);
void TWAI_sendStatoMacchina(uint8_t destinatario, uint8_t motore, uint8_t modalita, uint8_t pedale, uint8_t emergenza, uint8_t pid, uint8_t stato_s1, uint8_t funzione_s1);
void TWAI_sendReset();

#ifdef __cplusplus
}
#endif

#endif  // __TWAI_H__
