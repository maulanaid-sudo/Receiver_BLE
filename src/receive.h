#ifndef _RECEIVE_H_
#define _RECEIVE_H_s
void playBeep(void);
void handleBeep(void);
void checkTimeout(void);
void dataReceived(uint8_t* address, uint8_t* data, uint8_t len, signed int rssi, bool broadcast);
#endif