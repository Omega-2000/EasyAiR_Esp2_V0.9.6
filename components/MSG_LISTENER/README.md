# MSG_LISTENER

**DIPENDENZE**
- "PID.h" --> componente utilizzato per la gestione del funzionamento del pid
- "TWAI.h" --> componente utilizzato per la gestione dei messaggi canbus da inviare e da ricevere
- "EMERGENZE.h" --> componente utilizzato per la gestione dell'emergenze
- "PEDALE.h" --> componente utilizzato per la gestione delle modalità che utilizzano il pedale
- "CONFIG_FILE.h" --> componente utilizzato per la gestione dello spiff
- "SD.h" --> componente utilizzato per la gestione dell'sd
- "Updater.h" --> componente utilizzato per l'aggiornamento

**FUNZIONAMENTO**

Componente principale su cui si basa il funzionamenteo di tutte le funzionalità di questo microcontrollore (tranne il pid). La funzione "macchina_ready()" deve essere richiamata all'inizio del programma dopo l'inizializzazione di tutto per eseguire il ciclo d'avvio ed essere sicuro che la macchina possa essere funzionante. Invece la funzione "MSG_message_receiver()" deve essere richiamata ll'interno del loop principale per far interpretare correttamente i messaggi in arrivo ed eseguire le funzioni opportune in base alla richiesta ricevuta. Tutte le altre funzioni servono per gestire i millis dei 3 tempi di utilizzo salvati all'interno dello spiff e i file "values.txt" e "statistics.txt" salvati all'interno della scheda sd.

**CODICE**
- MSG_LISTENER.c
  - MSG_lettura_in_SPIFF();         //  legge i millis dei 3 tempi di utilizzo, li salva nelle variabili che incrementaranno e li stampa in seriale
  - macchina_ready();               //  si occupa dell'esecuzione del ciclo d'avvio della macchina
  - MSG_message_receiver();         //  riceve, interpreta i messaggi canbus in arrivo ed esegue le funzioni in base al messaggio ricevuto

- SD_OPERATIONS.c
  - MSG_valori_in_SD();             //  crea una stringa contenente tutti i valori da salvare e la salva all'interno dell'sd nel file "values.txt"
  - MSG_read_valori_in_SD();        //  legge i valori contenuti nel file "values.txt" nell'sd, li salva nelle variabili e li stampa in seriale
  - MSG_update_value_in_SD();       //  ogni 3 secondi esegue la funzione "MSG_valori_in_SD()"
  - SD_update_statistics();         //  crea una stringa contenente i minuti dei 3 tempi di utilizzo e la salva all'interno dell'sd nel file "statistics.txt"
  - SPIFF_update_statistics();      //  salva all'interno dello spiff i millis dei 3 tempi di utilizzo
  - millisToOreMsbLsb();            //  converte i millis dei 3 tempi di utilizzo in minuti, ore e li stampa in seriale