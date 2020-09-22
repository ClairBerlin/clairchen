#include "things_network.h"

// We need to be able to compile with different options without editing source.
// When building with a more advanced environment, set the following variable:
// ARDUINO_LMIC_PROJECT_CONFIG_H=my_project_config.h
// otherwise the lmic_project_config.h from the LMIC library folder will be used
#ifndef ARDUINO_LMIC_PROJECT_CONFIG_H
# define ARDUINO_LMIC_PROJECT_CONFIG_H /home/jan/Code/ClAir/clairchen/lmic_config.h
#endif

#include <arduino_lmic.h>
#include <hal/hal.h>
#include <Arduino.h>
#include "debug.h"

//--------------------------------------------------------------------
//--  Board Setup for Adafruit Feather MO LoRa
//--------------------------------------------------------------------
#if defined(ARDUINO_SAMD_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0)
// Pin mapping for Adafruit Feather M0 LoRa, etc.
const lmic_pinmap lmic_pins = {
  .nss = 8,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 4,
  .dio = {3, 6, LMIC_UNUSED_PIN},
  .rxtx_rx_active = 0,
  .rssi_cal = 8,              // LBT cal for the Adafruit Feather M0 LoRa, in dB
  // D-stepping of the silicon, so the 10Mhz SPI clock should work.
};
#else
# error "Unknown target"
#endif

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
