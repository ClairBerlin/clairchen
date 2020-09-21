#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

#define DEBUG 1 // enable (1) / disable (0) Serial prints

#if DEBUG

#define PRINT_INIT() do { \
    while (!Serial); \
    Serial.begin(9600); \
    Serial.println(F("Let's clean some air!")); \
  } while (0)
#define PRINT(ARG) Serial.print(ARG)
#define PRINTLN(ARG) Serial.println(ARG)
#define PRINT_RECEIVED_MSG() do { \
    PRINT(F("  Received downlink message with ")); \
    PRINT(LMIC.dataLen); \
    PRINTLN(F(" bytes of payload")); \
    PRINT(F("  RSSI [dBm]: ")); \
    PRINTLN(LMIC.rssi); \
    PRINT(F("  SNR [dB]: ")); \
    PRINTLN(LMIC.snr / 4); \
    u1_t bPort = 0; \
    if (LMIC.txrxFlags & TXRX_PORT) { \
      bPort = LMIC.frame[LMIC.dataBeg - 1]; \
      PRINT(F("  Received message on port ")); \
      PRINTLN(bPort); \
    } \
    if (LMIC.txrxFlags & TXRX_ACK) { \
      PRINTLN(F("  Received ack")); \
    } \
  } while (0)
#define PRINT_FLUSH() Serial.flush()

#else // DEBUG

#define PRINT_INIT() do {} while (0)
#define PRINT(ARG) do {} while (0)
#define PRINTLN(ARG) do {} while (0)
#define PRINT_RECEIVED_MSG() do {} while (0)
#define PRINT_FLUSH() do {} while (0)

#endif // DEBUG


#endif /* DEBUG_H */
