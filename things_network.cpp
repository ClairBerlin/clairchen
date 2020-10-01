#include "things_network.h"
#include "debug.h"
#include <arduino_lmic.h>
#include <hal/hal.h>
#include <Arduino.h>

#include "euis.h"

void os_getDevEui (u1_t* buf) {
  memcpy_P(buf, DEVEUI, 8);
}

void os_getArtEui (u1_t* buf) {
  memcpy_P(buf, APPEUI, 8);
}

void os_getDevKey (u1_t* buf) {
  memcpy_P(buf, APPKEY, 16);
}

// Set up the channel scheme used in the EU-868 band plan.
// They are fixed; hence, there is no need to store and reload them
static void setupEu868Channels() {
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;
}

void resumeConnection() {
  // Resume an already established connection - from a previous OTAA JOIN.

  uint8_t appskey[sizeof(APPSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));

  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));

  LMIC_setSession (0x13, DEVADDR, nwkskey, appskey);

  setupEu868Channels();

  PRINTLN(F("Resuming an already established session."));
  PRINT_ADDRESSES_AND_KEYS();
}
