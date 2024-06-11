# TWAI

**DIPENDENZE**
- "driver/twai.h" --> componente TWAI basato sul componente nativo di esp-idf "twai"
- "TWAI_DEFINE.h" --> file in cui sono definite le caratteristiche che dovranno avere i messaggi che verranno inviati, i gpio dei due pin che verranno usati per il canbus, gli ID e i primi byte dei messaggi che determinano l'argomento del messaggio e il comando specifico

**FUNZIONAMENTO**
Per far funzionare il twai serve inizialmente controllare di aver impostato i giusti gpio collegati alla scheda e inizializzare il twai tramite la funzione "TWAI_init()" all'inizio del programma. Successivamente basterà richiamre la funzione "TWAI_get()" per la ricezione dei messaggi e la funzione "TWAI_send()" per l'invio di messaggi.

**CODICE**
- ***FUNZIONI BASE PER GESTIRE LA COMUNICAZIONE CANBUS/TWAI***
- TWAI_init();                                                                  //  inizializza twai
- TWAI_reinit();                                                                //  reinizializza twai
- TWAI_send(uint32_t id, uint16_t flag, uint8_t len, uint8_t data[]);           //  invia messaggio
- TWAI_send_interrupt(uint32_t id, uint16_t flag, uint8_t len, uint8_t data[]); //  invia messaggio senza stampare niente in seriale, utile per essere utilizzata negli interrupt
- TWAI_get();                                                                   //  riceve messaggio
- TWAI_handle_rx_message(twai_message_t message);                               //  legge e memorizza messaggio (funzione static, quindi eseguita e richiamata solamente all'interno delle funzioni presenti nel componente TWAI)

- ***FUNZIONI DEDICATE ALL'INVIO DI UN SPECIFICO MESSAGGIO***
- TWAI_pingRequest(uint8_t destinatario);
- TWAI_pingReply(uint16_t versioneFW, uint8_t destinatario);
- TWAI_sendStatoPid(uint8_t stato, uint8_t temperatura);
- TWAI_sendConfermaEsportaOreInSd();
- TWAI_sendOreUtilizzo(uint8_t risposta, uint8_t ore_riscaldamento_MSB, uint8_t ore_riscaldamento_LSB, uint8_t ore_motore_MSB, uint8_t ore_motore_LSB, uint8_t ore_utilizzo_macchina_MSB, ore_utilizzo_macchina_LSB);
- TWAI_sendAzioneEmergenze(uint8_t stato, uint8_t azione);
- TWAI_sendAzioneEmergenze_interrupt(uint8_t stato, uint8_t azione);
- TWAI_sendStatoPedale(uint8_t stato, uint8_t azioni);
- TWAI_sendConfermaPedale();
- TWAI_sendConfermaTurtle();
- TWAI_sendConfermaS1();
- TWAI_sendConfermaElettrovalvole(uint8_t azione);
- TWAI_sendEmergenze(uint8_t fotocellula1, uint8_t fotocellula2, uint8_t fungo1, uint8_t fungo2, uint8_t pin_emergenze);
- TWAI_sendRichiestaValori(uint8_t tipo_valori);
- TWAI_sendValoriMotore(uint8_t destinatario, uint8_t v_min, uint8_t v_max, uint8_t v_reverse, uint8_t v_turtle, uint8_t v_impostata, uint8_t corrente_max);
- TWAI_sendValoriRiscaldamento1(uint8_t t_min, uint8_t t_max, uint8_t t_target);
- TWAI_sendValoriRiscaldamento2(uint8_t kp_, uint8_t ki_, uint8_t kd_, uint8_t offset_, uint8_t segno_offset_);
- TWAI_sendConfermaVelocita(uint8_t tipo_velocita, uint8_t valore);
- TWAI_sendMotorCommand(uint8_t comando);
- TWAI_sendError(uint8_t error);
- TWAI_sendStatoMacchina(uint8_t destinatario, uint8_t motore, uint8_t modalita, uint8_t pedale, uint8_t emergenza, uint8_t pid, uint8_t stato_s1, uint8_t funzione_s1);
- TWAI_sendReset();
