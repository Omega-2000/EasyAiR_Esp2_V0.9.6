# PEDALE

**DIPENDENZE**
- "TWAI.h" --> componente utilizzato per la gestione dei messaggi canbus da inviare e da ricevere
- "EMERGENZE.h" --> componente utilizzato per la gestione dell'emergenze
- "ELETTROVALVOLE.h" --> componente utilizzato per la gestione dell'elettrovalvole

**FUNZIONAMENTO**

La funzione "PEDALE_init()" deve essere richiamata all'inizio del programma per inizializzare i gpio dedicati al pedale e al relè. Invece richiamare la funzione "PEDALE_loop()" all'interno del loop principale dove si farà funzionare le modalità che utilizzano il pedale, perchè questa funzione ogni volta che verrà eseguita controllerà lo stato di alcuni flag e sceglierà stato, modalità e funzione del pedale.

**CODICE**
- PEDALE_init();  //  inizializza gpio del pedale e del relè
- PEDALE_loop();  //  gestisce il funzionamento delle modalità pedale, tartaruga ed s1

**MODALITA'**
- PEDALE     -->  modalità che quando verrà selezionata farà fermare il motore in qualsiasi modalità sia e passerà il comando del motore al pedale (se premuto il motore girerà, mentre se rilasciato il motore si fermerà e rimarrà fermo), (per disattivare la modalità, cliccare nuovamente il pulsante "pedale" con cui si ha attivato la modalità)
- TARTARUGA  -->  modalità che quando verrà selezionata farà fermare il motore in qualsiasi modalità sia e passerà il comando del motore al pedale ma in velocità tartargua impostata dal menù tecnico nella sezione motore, (per disattivare la modalità, cliccare nuovamente il pulsante "tartaruga" con cui si ha attivato la modalità)
- S1         -->  modalità che quando viene selezionata, permetterà di premere il pedale per eseguire la funzione "abbassa" e una volta conclusa la funzione, si disattiverà in automatico la modalità S1