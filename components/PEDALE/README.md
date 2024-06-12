# PEDALE

**DIPENDENZE**
- "TWAI.h" --> componente utilizzato per la gestione dei messaggi canbus da inviare e da ricevere
- "EMERGENZE.h" --> componente utilizzato per la gestione dell'emergenze
- "ELETTROVALVOLE.h" --> componente utilizzato per la gestione dell'elettrovalvole

**FUNZIONAMENTO**

Chiamare la funzione "PEDALE_init()" all'inizio del programma per inizializzare i gpio dedicati al pedale e al relè

**CODICE**
- PEDALE_init();  //  inizializza gpio del pedale e del relè
- PEDALE_loop();  //  gestisce il funzionamento delle modalità pedale, tartaruga ed s1