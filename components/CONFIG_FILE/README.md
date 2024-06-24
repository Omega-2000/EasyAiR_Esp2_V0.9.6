# CONFIG_FILE

**DIPENDENZE**
- "PID.h"

**FUNZIONAMENTO**

Serve richiamare la funzione "SPIFFS_init()" all'inizio del programma per poter utilizzare lo spiffs. Successivamente si potrà utilizzare lo spiffs richiamando le altre 3 funzioni di questo componente.

**CODICE**
- SPIFFS_init();        //  inizializza spiffs e monta il file system all'interno della memoria del microcontrollore
- SPIFFS_write_();      //  crea una stringa contenente i millis dei 3 tempi di utilizzo e la salva all'interno del file "millis.csv" nello spiffs
- SPIFFS_read_();       //  legge la stringa all'interno del file "millis.csv", divide i 3 valori in millis, li salva nelle variabili che incrementeranno e li stampa in seriale
- SPIFFS_remove_();     //  rimuove un file all'interno dello spiffs e controlla l'effettiva rimozione del file
