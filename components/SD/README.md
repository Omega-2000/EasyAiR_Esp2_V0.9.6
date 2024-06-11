# SD

**DIPENDENZE**
- "esp_vfs_fat.h" --> componente utilizzato per la gestione del file system: per il montaggio del file system e la gestione dei file all'interno del file system
- "sdmmc_cmd.h" --> componente utilizzato per la gestione della scheda sd: inzializza la scheda sd e legge le caratteristiche della scheda
- "dirent.h" --> componente utilizzato per la gestione delle cartelle all'interno del file system

**FUNZIONAMENTO**

All'inizio del programma dovrà essere chiamata la funzione "SD_init()" che inizializzerà l'SPI, la scheda sd e il file system. Successivamente si potrà controllare la presenza o meno di un file chiamando la funzione "SD_check", oppure si potrà scrivere all'interno dei file e leggere i file eseguendo la funzione "SD_operation".

**CODICE**
- SD_init(Sd* sd, uint8_t miso, uint8_t mosi, uint8_t clk, uint8_t cs);  //  inizializza la scheda sd e il file system
- SD_SPI_begin(Sd* sd);  //  inizializza SPI (funzione chiamata all'interno della funzione SD_init())
- SD_check(Sd* sd, const char *path, const char *object);  //  controlla e ritorna se il file esiste oppure no
- SD_write(Sd* sd, uint8_t num_file, const char *string);  //  scrive una stringa all'interno di un file
- SD_read(Sd* sd, uint8_t num_file);  //  legge un file
- SD_listDir(Sd* sd, const char *path);  //  mostra il contenuto di una cartella
- SD_operation(Sd* sd, bool operation, uint8_t n_file, char *s);  //  funzione utilizzata per eseguire la funzione "SD_write" o la funzione "SD_read"

**CONTENUTO SCHEDA SD**
- updates
  - Display
    - Data
    - firmware.bin
  - Driver
    - firmware.bin
  - Self
    - firmware.bin
  - versions.txt

- statistics.txt        -->     file contenente i minuti di utilizzo della macchina, del motore e del pid

- temperatures.csv      -->     file contenente un log del funzionamento del pid

- values.txt
