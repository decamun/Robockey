#define PACKET_LENGTH 10

char buffer[PACKET_LENGTH];
int message;
void rf_comm(char* buffer);
ISR(INT2_vect);