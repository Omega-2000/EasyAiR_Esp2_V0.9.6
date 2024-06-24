# CAN_file_transfer

**DIPENDENZE**
- "TWAI.h" --> componente utilizzato per la gestione dei messaggi canbus da inviare e da ricevere

**FUNZIONAMENTO**
Componente composto da funzioni che sono utilizzate per il trasferimento dei file dell'aggiornamento

**CODICE**
- CANFileTransfer * createNewCanFileTransfer();
- CAN_FT_messageListner(void *pvParameters);
- CAN_FT_initCanbus();
- CAN_FT_sendMessage(uint32_t id, uint8_t data[]);
- checkSpiffsFreeSpace();
- analyzeMessagesFileReceiver(twai_message_t message);
- analyzeMessagesFileSender(twai_message_t message);
- waitResponse(int timeout);
- isExistingTransferProcess();
- createCANReceiverTask();
- deleteCANReceiverTask();
- CAN_sendChunk(uint8_t id, uint8_t *chunk);
- CAN_endChunk(uint8_t status);
- CAN_sendFileNotification(uint8_t dest);
- CAN_sendAction(uint8_t action);
- CAN_initChunkTransmission(int32_t totalChunkNbr, uint32_t filesize);
- CAN_sendFileName(uint8_t *data);
- CAN_sendFileTransfertInfo(uint8_t *device,uint8_t status);
- CAN_resendAction();
- divideFileIntoChunks(const char *filename);
- senFilename(uint8_t dest,const char *filename);
- calculateFileCrc32(const char *filename);
- (*UpdateStateChangedHandler)(int device, int state);
- registerUpdateStateChangedHandler(UpdateStateChangedHandler handler);
- processUpdateState(int device, int state);
