#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

#ifndef DEBUG
#define DEBUG 1 // enable (1) / disable (0) Serial prints
#endif

#if DEBUG

#define PRINT_INIT() do { \
    while (!Serial); \
    Serial.begin(9600); \
    Serial.println(F("Let's clean some air!")); \
} while (0)

#define PRINT_FLUSH() Serial.flush()

#define PRINT(ARG) Serial.print(ARG)

#define PRINT_HEX(ARG) Serial.print(ARG, HEX)

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

#define PRINT_KEY(KEY) do { \
  for (int i = 0; i < sizeof(KEY); i++) { \
    if (i != 0) PRINT("-"); \
    if (KEY[i] < 16) PRINT(0); \
    PRINT_HEX(KEY[i]); \
  } \
} while (0)

#define PRINT_ADDRESSES_AND_KEYS() do { \
  u4_t netid = 0; \
  devaddr_t devaddr = 0; \
  u1_t nwkKey[16]; \
  u1_t artKey[16]; \
  LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey); \
  PRINT(F("netid: ")); PRINTLN(netid); \
  PRINT(F("devaddr: ")); PRINT_HEX(devaddr); PRINTLN(""); \
  PRINT(F("AppSKey: ")); PRINT_KEY(artKey); PRINTLN(""); \
  PRINT(F("NwSKey: ")); PRINT_KEY(nwkKey); PRINTLN(""); \
  PRINT_FLUSH(); \
} while (0)

#else // DEBUG

#define PRINT_INIT() do {} while (0)
#define PRINT_FLUSH() do {} while (0)
#define PRINT(ARG) do {} while (0)
#define PRINT_HEX(ARG) do {} while (0)
#define PRINTLN(ARG) do {} while (0)
#define PRINT_RECEIVED_MSG() do {} while (0)
#define PRINT_ADDRESSES_AND_KEYS() do {} while (0)

#endif // DEBUG


#endif /* DEBUG_H */
