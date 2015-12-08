//RF COMM
void distributePacket(char* out_buffer);
void handleRfGamestate(uint8_t value);
void handleRfCommand(char* rf_buffer);
void handleRfRobotInfo(char* rf_buffer);
void sendRfRobotInfo();
void handleRfPuckLocation(char* rf_buffer);
int validateString(uint8_t rf_passcode);
