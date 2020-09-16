/*******************************************************************************
   Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
   Copyright (c) 2018 Terry Moore, MCCI

   Permission is hereby granted, free of charge, to anyone
   obtaining a copy of this document and accompanying files,
   to do whatever they want with them without any restriction,
   including, but not limited to, copying, modification and redistribution.
   NO WARRANTY OF ANY KIND IS PROVIDED.

   This example sends a valid LoRaWAN packet with payload "Hello,
   world!", using frequency and encryption settings matching those of
   the The Things Network. It's pre-configured for the Adafruit
   Feather M0 LoRa.

   This uses OTAA (Over-the-air activation), where where a DevEUI and
   application key is configured, which are used in an over-the-air
   activation procedure where a DevAddr and session keys are
   assigned/generated for use with all further communication.

   Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
   g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
   violated by this sketch when left running for longer)!

   To use this sketch, first register your application and device with
   the things network, to set or generate an AppEUI, DevEUI and AppKey.
   Multiple devices can use the same AppEUI, but each device has its own
   DevEUI and AppKey.

   Do not forget to define the radio type correctly in
   arduino-lmic/project_config/lmic_project_config.h or from your BOARDS.txt.

 *******************************************************************************/

// We need to be able to compile with different options without editing source.
// When building with a more advanced environment, set the following variable:
// ARDUINO_LMIC_PROJECT_CONFIG_H=my_project_config.h
// otherwise the lmic_project_config.h from the LMIC library folder will be used
#ifndef ARDUINO_LMIC_PROJECT_CONFIG_H
# define ARDUINO_LMIC_PROJECT_CONFIG_H /home/jan/Code/ClAir/clairchen/project_config/lmic_project_config.h
#endif

#include "clair.h"

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h"
SCD30 airSensor;

// Define to resume an existing session without JOINING the TTN again.
// Requires DeviceAddr, NwSKey, and AppSKey to be defined below.
#ifndef CONN_RESUME
# define CONN_RESUME 0
#endif

#include "euis.h"

void os_getArtEui (u1_t* buf) {
  memcpy_P(buf, APPEUI, 8);
}

void os_getDevEui (u1_t* buf) {
  memcpy_P(buf, DEVEUI, 8);
}

void os_getDevKey (u1_t* buf) {
  memcpy_P(buf, APPKEY, 16);
}

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

static osjob_t sendjob;
const unsigned TX_INTERVAL = 60;

// Set up the channel scheme used in the EU-868 band plan.
// They are fixed; hence, there is no need to store and reload them
void setupEu868Channels() {
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

//--------------------------------------------------------------------
//--  Debug Utility Functions
//--------------------------------------------------------------------
void printHex2(unsigned v) {
  v &= 0xff;
  if (v < 16)
    Serial.print('0');
  Serial.print(v, HEX);
}

void printAddrAndKeys() {
  u4_t netid = 0;
  devaddr_t devaddr = 0;
  u1_t nwkKey[16];
  u1_t artKey[16];
  LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
  Serial.print(F("netid: "));
  Serial.println(netid, DEC);
  Serial.print(F("devaddr: "));
  Serial.println(devaddr, HEX);
  Serial.print(F("AppSKey: "));
  for (size_t i = 0; i < sizeof(artKey); ++i) {
    if (i != 0)
      Serial.print("-");
    printHex2(artKey[i]);
  }
  Serial.println("");
  Serial.print(F("NwkSKey: "));
  for (size_t i = 0; i < sizeof(nwkKey); ++i) {
    if (i != 0)
      Serial.print("-");
    printHex2(nwkKey[i]);
  }
  Serial.println();
  Serial.flush();
}

void printResumeInfo() {
  Serial.println(F("Resuming an already established session."));
  printAddrAndKeys();
}

void printDatarate() {
  Serial.print(" current datarate: ");
  Serial.print(LMIC.datarate);
  Serial.print("/");
  switch (LMIC.datarate) {
    case  DR_SF12:
      Serial.println("DR_SF12");
      break;
    case DR_SF11:
      Serial.println("DR_SF11");
      break;
    case DR_SF10:
      Serial.println("DR_SF10");
      break;
    case DR_SF9:
      Serial.println("DR_SF9");
      break;
    case DR_SF8:
      Serial.println("DR_SF8");
      break;
    case DR_SF7:
      Serial.println("DR_SF7");
      break;
    case DR_SF7B:
      Serial.println("DR_SF7B");
      break;
    case DR_FSK:
      Serial.println("DR_FSK");
      break;
    default:
      Serial.println("DR_NONE");
  }
}

//--------------------------------------------------------------------
//--  Main TX/RX Event Handler
//--------------------------------------------------------------------
void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      printAddrAndKeys();
      // Enable automatic data rate adjustment
      LMIC_setAdrMode(1);
      break;
    /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_RFU1:
      ||     Serial.println(F("EV_RFU1"));
      ||     break;
    */
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      // Any data to be received?
      if (LMIC.dataLen != 0 || LMIC.dataBeg != 0) {
        Serial.print(F("  Received downlink message with "));
        Serial.print(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
        Serial.print(F("  RSSI [dBm]: "));
        Serial.println(LMIC.rssi);
        Serial.print(F("  SNR [dB]: "));
        Serial.println(LMIC.snr / 4);
        // Data was received. Extract port number if any.
        u1_t bPort = 0;
        if (LMIC.txrxFlags & TXRX_PORT) {
          bPort = LMIC.frame[LMIC.dataBeg - 1];
          Serial.print(F("  Received message on port "));
          Serial.println(bPort);
        }
        if (LMIC.txrxFlags & TXRX_ACK) {
          Serial.println(F("  Received ack"));
        }
        // Call user-supplied function with port #, pMessage, nMessage; // nMessage might be zero.
        // receiveMessage(bPort, LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
      } else {
        Serial.println(F("  Did not receive any downlink message."));
      }
      Serial.print("  Uplink Sequence Counter: ");
      Serial.println(LMIC.seqnoUp, DEC);
      Serial.print("  Downlink Sequence Counter: ");
      Serial.println(LMIC.seqnoDn, DEC);
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_SCAN_FOUND:
      ||    Serial.println(F("EV_SCAN_FOUND"));
      ||    break;
    */
    case EV_TXSTART:
      Serial.println(F("EV_TXSTART"));
      break;
    case EV_TXCANCELED:
      Serial.println(F("EV_TXCANCELED"));
      break;
    case EV_RXSTART:
      /* do not print anything -- it wrecks timing */
      break;
    case EV_JOIN_TXCOMPLETE:
      Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
      break;

    default:
      Serial.print(F("Unknown event: "));
      Serial.println((unsigned) ev);
      break;
  }
  Serial.flush();
}

clair_sample_t sampleMeasurements() {
  clair_sample_t sample;

  sample.co2ppm = airSensor.getCO2();
  sample.temperature = airSensor.getTemperature();
  sample.humidity = airSensor.getHumidity();

  Serial.print("co2(ppm):");
  Serial.print(sample.co2ppm);

  Serial.print(" temp(C):");
  Serial.print(sample.temperature, 1);

  Serial.print(" humidity(%):");
  Serial.print(sample.humidity, 1);

  return sample;
}

void encodeSample(clair_sample_t sample, uint8_t *byteBuffer) {
#define CLAIR_PROTOCOL_VERSION 0
#define CLAIR_MESSAGE_ID_SAMPLE_LIST 0
#define CLAIR_MESSAGE_HEADER_SAMPLE_LIST 0

  byteBuffer[0] = 0;
  byteBuffer[0] |= CLAIR_PROTOCOL_VERSION << 5; // protocol version 1 (3 bits)
  byteBuffer[0] |= CLAIR_MESSAGE_ID_SAMPLE_LIST << 3; // message identifier (2 bits)
  byteBuffer[0] |= CLAIR_MESSAGE_HEADER_SAMPLE_LIST; // message header (3 bits)

  byteBuffer[1] = (sample.co2ppm + 10) / 20;

  byteBuffer[2] = 0;
  byteBuffer[2] |= (((uint8_t) (sample.temperature + 0.5)) & 0x1F) << 3; // 5 bits
  byteBuffer[2] |= ((uint8_t) (sample.humidity + 5) / 10) & 0x7; // 3 bits
}

void do_send(osjob_t* j) {
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    clair_sample_t sample = sampleMeasurements();
    uint8_t byteBuffer[3];

    encodeSample(sample, byteBuffer);

    LMIC_setTxData2(1, byteBuffer, 3, 0);

    Serial.println(F("Packet queued"));

    printDatarate();
  }
}
// Next TX is scheduled after TX_COMPLETE event.

void resumeConnection() {
  // Resume an already established connection - from a previous OTAA JOIN.
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession (0x13, DEVADDR, nwkskey, appskey);
  setupEu868Channels();
  // Start out with maximum data rate and power. Works in Berlin because of dense TTN coverage.
  LMIC_setDrTxpow(DR_SF12, 14);
  // Enable automatic data rate adjustment
  LMIC_setAdrMode(0);
  printResumeInfo();
}

void initSensor() {
  Wire.begin();
  if (airSensor.begin() == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1)
      ;
  }
}

void setup() {
  delay(5000);
  while (! Serial)
    ;
  Serial.begin(9600);
  Serial.println(F("Starting"));

  initSensor();

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

#if CONN_RESUME
  resumeConnection();
#endif

  LMIC_setDrTxpow(DR_SF12, 14);
  LMIC_setAdrMode(0);

  Serial.flush();

  // Start job (sending automatically starts OTAA too)
  do_send(&sendjob);
}

void loop() {
  os_runloop_once();
}
