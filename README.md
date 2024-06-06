# EasyAiR_Esp2_V0.9.6

**HARDWARE**
- Scheda EasyAiR V0.7.03.24
- Microcontrollore chip ESP32-S3FN8
- Collegarsi all'usb 2 o al pin header n.ro 2 (quelli più in basso, verso il driver)
- Alimentazione 24V necessaria per poter utilizzare tutte le funzionalità della scheda, altrimenti alimentazione 5V per poter utilizzare tutto ciò che richiede fino a 5V

**REQUISITI PER IL FUNZIONAMENTO**
- La scheda deve essere montata e collegata correttamente a tutti gli ingressi e a tutte le uscite tramite connettori presenti sulla scheda
- La scheda EasyAiR deve essere montata con una scheda Driver
- Entrambi gli altri due microcontrollori (ESP-Display e ESP-Driver) devono essere programmati correttamente col proprio FW

**AMBIENTE DI SVILUPPO**
- Framework: ESP-IDF v5.0.2

**LINGUAGGIO DI PROGRAMMAZIONE**
- C

**COMPONENTI**
- CAN_file_transfer
  - Componente utilizzato per il trasferimento file dell'aggiornamento del FW.

- CONFIG_FILE
  - Componente utilizzato per gestire le operazioni all'interno dello spiff.

- ELETTROVALVOLE
  - Componente utilizzato per inizializzare i gpio a cui saranno collegate le due elettrovalvole e per spegnerle dopo averle attivate.

- EMERGENZE
  - Componente utilizzato per la gestione di tutte l'emergenze collegate alla scheda.

- MSG_LISTENER
  - Componente utilizzato per il ciclo d'avvio, per la gestione di tutti i messaggi canbus in arrivo, per la scrittura e la lettura dei valori in SD e infine per la scrittura e la lettura dei valori delle statistiche d'utilizzo all'interno dello spiff.

- PEDALE
  - Componente utilizzato per gestire il pedale in tutte le modalità in cui può essere usato.

- PID
  - Componente utilizzato per gestire il funzionamento del PID e i suoi valori, per leggere la temperatura dal sensore di temperatura, per gestire il funzionamento del watchdog e infine per scrivere all'interno del file "temperatures.csv" in sd.

- SD
  - Componente utilizzato per gestire le operazioni all'interno dell'sd.

- TWAI
  - Componente utilizzato per gestire la comunicazione canbus, riceve e invia messaggi.

- Updater
  - Componente utilizzato per la gestione dell'aggiornamento del FW.

**FUNZIONAMENTO IN SINTESI**
- Il programma parte inizializzando tutti gli ingressi, le uscite, il twai, l'i2c, il pid, lo spiff e infine l'sd. Successivamente farà partire un task sul core 0 (taskMain), un task sul core 1 (taskPid) e infine un task che lavorerà in entrambi i core (taskUpdate). Quest'ultimo non farà niente fino a che non si riceverà il messaggio di inizio aggiornamento, in quel momento farà partire la routine di aggiornamento per il microcontrollore su cui è caricato questo programma e poi invierà il FW aggiornato anche agli altri due microcontrollori della scheda. Il "taskPid" aspetta che il ciclo d'avvio venga completato correttamente, una volta completato il task partirà e continuerà sempre a controllare il funzionamento del pid, invierà la temperatura letta dal sensore al microcontrollore del display e la scriverà anche all'interno del file "temperatures.csv" ogni secondo. Lo stesso task inizializzerà il watchdog che controllerà che il ciclo di funzionamento del pid non si blocchi e riavvierà il pid nel caso in cui sia stato fermato dallo scattare di uno dei funghi d'emergenza. Mentre il "taskMain" inizializza gli interrupt dell'emergenze, esegue il test delle fotocellule, invia il messaggio iniziale di ping agli altri due microcontrollori e avvia il ciclo d'avvio. Se questo viene completato correttamente, allora si procederà all'esecuzione del programma vero e proprio. Come prima cosa inizializza i vari timer e imposta i valori del pid salvati nell'sd, successivamente partirà la routine del programma principale. Questa routine controlla qualsiasi messaggio in entrata e fa partire una certa funzione a seconda del comando ricevuto, controlla lo scattare dell'emergenze collegate alla scheda, controlla il funzionamento del pedale e delle elettrovalvole, infine aggiorna all'interno dell'sd i valori salvati e nello spiff le statistiche d'utilizzo.

**FUNZIONAMENTO CODICE (main.c)**
- inizializzazione()
  - La prima funzione che viene eseguita dal programma. Questa funzione inizializza gli ingressi, le uscite, il twai, l'i2c, il pid, lo spiff e l'sd.

- taskMain(void *pvParameters)
  - Questo è il task principale in cui vengono eseguite la maggior parte delle funzioni di questa scheda.

- taskPid(void *pvParameters)
  - Questo task invece viene dedicato al funzionamento del pid e a tutto quello che lo riguarda.

- taskUpdate(void *pvParameters)
  - Questo task viene utilizzato dal microcontrollore al momento dell'aggiornamento.

- app_main(void)
  - All'interno del main viene stampata la versione del FW, eseguita la funzione "inizializzazione()" e poi vengono installati i task. Il "taskMain" viene eseguito nel core 0 del microcontrollore, il "taskPid" viene eseguito nel core 1 e il "taskUpdate" viene eseguito su entrambi i core e si attiverà al momento della ricezione del comando di avvio aggiornamento.

**Differenze tra questo FW e quello precedente**
- Questo FW è stato sviluppato per poter funzionare con le modifiche effettuate sulla versione HW V0.7.03.24. Per cui ha una gestione differente dell'emergenze.


AGGIUNGERE I README SU TUTTI I COMPONENTI PER DESCRIVERE LE VARIE FUNZIONI......