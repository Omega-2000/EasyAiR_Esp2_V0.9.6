# Updater

**DIPENDENZE**
- "esp_ota_ops.h" --> componente utilizzato per la gestione della partizione "ota" (partizione utilizzata per l'aggiornamento)
- "esp_partition.h" --> componente utilizzato per la gestione delle partizioni all'interno del file system del microcontrollore
- "CAN_file_transfer.h" --> componente utilizzato la gestione dei messaggi canbus che servono all'invio dell'aggiornamento agli altri due microcontrollori
- "TWAI.h" --> componente utilizzato per la gestione dei messaggi canbus da inviare e da ricevere

**FUNZIONAMENTO**

La funzione principale di questo componente è "performUpdate()" che eseguirà l'aggiornamento su questo microcontrollore, e poi invierà gli altri due aggiornamenti ai rispettivi microcontrollori.

**CODICE**
- getFirmwareVersions();  //  legge e salva le versioni dei firmware dal file "versions.txt" presente all'interno della scheda sd
- getUpdateInfo();  //  stampa in seriale la grandezza dell'aggiornamento
- checkupdate();  //  controlla se ci sono degli aggiornamenti da applicare e ritorna un valore booleano a seconda dell'esito del controllo
- updateSpiffs(uint8_t device);  //  controlla se lo spiffs deve essere aggiornato e ritorna un valore booleano a seconda dell'esito del controllo
- perform_ota_update(const char *file_path);  //  applica e installa l'aggiornamento appena scaricato
- getFileSize(const char* filePath);  //  ritorna la grandezza del file passato come parametro
- performUpdate();  //  esegue l'aggiornamento di tutti e tre i microcontrollori
- TWAI_sendUpdateSize(uint32_t filesize);  //  invia un messaggio contenente la grandezza dell'aggiornamento
- TWAI_sendUpdateInfo(Version* version);  //  invia un messaggio contenente le informazioni dell'aggiornamento
