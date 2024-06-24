# EMERGENZE

**DIPENDENZE**
- "TWAI.h" --> componente utilizzato per la gestione dei messaggi canbus da inviare e da ricevere
- "PID.h" --> componente utilizzato per la gestione del pid
- "ELETTROVALVOLE.h" --> componente utilizzato per la gestione dell'elettrovalvole

**FUNZIONAMENTO**
Gli interrupt dell'emergenze e del pedale scatteranno in automatico se inizializzati prima con la funzione "EM_configure_gpio_interrupt()". Richiamare la funzione "EM_test_fotocellule()" per eseguire il test della fotocellula e la funzione "EM_loop()" all'interno del loop principale che gestirà le "azioni emergenze" tramite la funzione "EM_start_azione()" e tutto il resto.

**CODICE**
- IRAM_ATTR EM_emergency(bool funghi);          //  funzione di interrupt 
- IRAM_ATTR EM_fotocellula1(void* arg);         //  funzione di interrupt 
- IRAM_ATTR EM_fungo1(void* arg);               //  funzione di interrupt 
- IRAM_ATTR EM_fungo2(void* arg);               //  funzione di interrupt 
- IRAM_ATTR EM_reset(void* arg);                //  funzione di interrupt 
- IRAM_ATTR EM_pedale(void* arg);               //  funzione di interrupt 
- EM_configure_gpio_interrupt();                //  inizializza i gpio utilizzati in questo componente
- EM_test_fotocellule();                        //  esegue il test della fotocellula
- EM_start_azione(int8_t azione);               //  invia lo stop della vecchia "azione emergenza" e lo start di quella nuova
- EM_start_azione_interrupt(int8_t azione);     //  invia lo stop della vecchia "azione emergenza" e lo start di quella nuova senza stampare in seriale, utile da utilizzare negli interrupt
- EM_loop();                                    //  gestisce il cambio di emergenza anche senza premere il reset, controlla se ci sono emergenze all'avvio della macchina, controlla se viene interrotta la fotocellula durante l'esecuzione della funzione "abbassa", controlla se l'emergenze sono state veramente resettate dopo aver premuto il pulsante "reset", gestisce le "azioni emergenza" e infine si occupa del scrivere un paio di messaggi nel log all'interno del file "temperatures.csv"

**EMERGENZE**
- FUNGO 1
- FUNGO 2
- FOTOCELLULA
- (RESET)

**AZIONI EMERGENZE**
- [1] BLINK LED ROSSO DEL RESET
- [2] BLINK LED ROSSO DEL RESET + BUZZER
- [3] BLINK LED ROSSO DEL RESET + BUZZER CON FREQUENZA RADDOPPIATA