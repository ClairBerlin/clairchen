/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 * Copyright (c) 2018 Terry Moore, MCCI
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network. It's pre-configured for the Adafruit
 * Feather M0 LoRa.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * Do not forget to define the radio type correctly in
 * arduino-lmic/project_config/lmic_project_config.h or from your BOARDS.txt.
 *
 *******************************************************************************/

// We need to be able to compile with different options without editing source.
// When building with a more advanced environment, set the following variable:
// ARDUINO_LMIC_PROJECT_CONFIG_H=my_project_config.h
// otherwise the lmic_project_config.h from the LMIC library folder will be used
#ifndef ARDUINO_LMIC_PROJECT_CONFIG_H
# define ARDUINO_LMIC_PROJECT_CONFIG_H ../../../clair-ttn-proto1/project_config/lmic_project_config.h
#endif

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <FlashStorage.h>


// Create a structure that is big enough to hold the state of the LMI
// engine such that it can continue to communicate with an already
// joined LoRaWAN network.
typedef struct {
  boolean valid; // True means that the struct has been written previously
  u4_t netid;
  devaddr_t devaddr;
  u1_t nwkKey[16];
  u1_t artKey[16];
  u4_t seqnoUp;
  u4_t seqnoDn;
  u1_t dn2Dr;
  u1_t dnConf;
  u1_t txChnl;
  s1_t datarate;
  s1_t adrTxPow;
  s2_t adrAckReq;
  u2_t devNonce;
  band_t bands[MAX_BANDS];
  u4_t channelFreq[MAX_CHANNELS];
  u4_t channelDlFreq[MAX_CHANNELS];
  u2_t channelDrMap[MAX_CHANNELS];
  u2_t channelMap;
  u1_t rx1DrOffset;
  u1_t rxDelay;
} LMIC_Connection_State;

// Reserve a portion of flash memory to store a LMIC_Connection_State
// and call it "my_connection_store".
// Note: the area of flash memory reserved lost every time
// the sketch is uploaded on the board.
FlashStorage(my_connection_store, LMIC_Connection_State);


// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8] = { 0xBA, 0x47, 0x03, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = { 0xE0, 0x93, 0x11, 0x00, 0x00, 0xB6, 0x76, 0x98 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from the TTN console can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0x3E, 0x80, 0x01, 0x93, 0x8E, 0x2D, 0x42, 0x27, 0x97, 0x9C, 0xE3, 0x6E, 0x2A, 0x4F, 0x2C, 0x17 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

// LoRaWAN NwkSKey, network session key -- To directly connect without performing a JOIN
// This should be in big-endian (aka msb).
// Key for Mrs. Red
static const PROGMEM u1_t NWKSKEY[16] = { 0xC3, 0x53, 0x49, 0x32, 0x89, 0xA5, 0xC6, 0x3D, 0x0A, 0x1E, 0x9A, 0x58, 0xD4, 0x75, 0xF4, 0xBE };

// LoRaWAN AppSKey, application session key -- To directly connect without performing a JOIN
// This should also be in big-endian (aka msb).
// Key for Mrs. Red
static const u1_t PROGMEM APPSKEY[16] = { 0x80, 0x16, 0x9D, 0xC3, 0x3C, 0x88, 0x72, 0xB5, 0x60, 0x5A, 0xDA, 0x35, 0xA7, 0xF1, 0x3B, 0x28 };

// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
// The library converts the address to network byte order as needed, so this should be in big-endian (aka msb) too.
static const u4_t DEVADDR = 0x2601595B ; // Address for Mrs. Red


// Program Version, to be printed in debug messages.
// Increment with each change to the code and commit.
static const u2_t progVer = 2;

static uint8_t mydata[] = "H";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

// Pin mapping
//
// Adafruit BSPs are not consistent -- m0 express defs ARDUINO_SAMD_FEATHER_M0,
// m0 defs ADAFRUIT_FEATHER_M0
//
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

// Store the LMIC connection state in on-chip Flash memory,
// so we can continue to communicate with the network following
// a reboot without the need to perform a JOIN.
// See https://www.thethingsnetwork.org/forum/t/how-to-persist-lmic-otaa-parameters-with-an-esp32/35032/12 for details.
void persistConnectionState() {
  Serial.println(F("Persist LMIC connection state to Flash ..."));

  // Temporary variable that gets persisted in Flash later on.
  LMIC_Connection_State connState;

  // Assmble the state to be persisted.
  connState.valid = true;
  // netid
  connState.netid = LMIC.netid;
  Serial.println("LMIC and connState netid: ");
  Serial.println(LMIC.netid, DEC);
  Serial.println(connState.netid, DEC);
  // devaddr
  connState.devaddr = LMIC.devaddr;
  Serial.println("LMIC and connState devaddr: ");
  Serial.println(LMIC.devaddr, HEX);
  Serial.println(connState.devaddr, HEX);
  // nwkKey
  memcpy(connState.nwkKey, LMIC.nwkKey, 16);
  Serial.print("LMIC.nwkKey: ");
  for (size_t i=0; i<sizeof(LMIC.nwkKey); ++i) {
          if (i != 0)
                  Serial.print("-");
          printHex2(LMIC.nwkKey[i]);
  }
  Serial.println();
  Serial.print("connState.nwkKey: ");
  for (size_t i=0; i<sizeof(connState.nwkKey); ++i) {
          if (i != 0)
                  Serial.print("-");
          printHex2(connState.nwkKey[i]);
  }
  Serial.println();
  // artKey
  memcpy(connState.artKey, LMIC.artKey, 16);
  Serial.print("LMIC.artKey: ");
  for (size_t i=0; i<sizeof(LMIC.artKey); ++i) {
          if (i != 0)
                  Serial.print("-");
          printHex2(LMIC.artKey[i]);
  }
  Serial.println();
  Serial.print("connState.artKey: ");
  for (size_t i=0; i<sizeof(connState.artKey); ++i) {
          if (i != 0)
                  Serial.print("-");
          printHex2(connState.artKey[i]);
  }
  Serial.println();
  // seqnoUp
  connState.seqnoUp = LMIC.seqnoUp;
  Serial.println("LMIC and connState seqnoUp: ");
  Serial.println(LMIC.seqnoUp, DEC);
  Serial.println(connState.seqnoUp, DEC);
  // seqnoDn
  connState.seqnoDn = LMIC.seqnoDn;
  Serial.println("LMIC and connState seqnoDn: ");
  Serial.println(LMIC.seqnoDn, DEC);
  Serial.println(connState.seqnoDn, DEC);
  // dn2Dr
  connState.dn2Dr = LMIC.dn2Dr;
  Serial.println("LMIC and connState dn2Dr: ");
  Serial.println(LMIC.dn2Dr, DEC);
  Serial.println(connState.dn2Dr, DEC);
  // dnConf
  connState.dnConf = LMIC.dnConf;
  Serial.println("LMIC and connState dnConf: ");
  Serial.println(LMIC.dnConf, DEC);
  Serial.println(connState.dnConf, DEC);
  // txChnl
  connState.txChnl = LMIC.txChnl;
  Serial.println("LMIC and connState txChnl: ");
  Serial.println(LMIC.txChnl, DEC);
  Serial.println(connState.txChnl, DEC);
  // datarate
  connState.datarate = LMIC.datarate;
  Serial.println("LMIC and connState datarate: ");
  Serial.println(LMIC.datarate, DEC);
  Serial.println(connState.datarate, DEC);  
  // adrTxPow
  connState.adrTxPow = LMIC.adrTxPow;
  Serial.println("LMIC and connState adrTxPow: ");
  Serial.println(LMIC.adrTxPow, DEC);
  Serial.println(connState.adrTxPow, DEC);
  // adrAckReq
  connState.adrAckReq = LMIC.adrAckReq;
  Serial.println("LMIC and connState adrAckReq: ");
  Serial.println(LMIC.adrAckReq, HEX);
  Serial.println(connState.adrAckReq, HEX);
  // devNonce
  connState.devNonce = LMIC.devNonce;
  Serial.println("LMIC and connState devNonce: ");
  Serial.println(LMIC.devNonce, HEX);
  Serial.println(connState.devNonce, HEX);
  // bands
  memcpy(connState.bands, LMIC.bands, MAX_BANDS*sizeof(band_t));
  // channelFreq
  memcpy(connState.channelFreq, LMIC.channelFreq, MAX_CHANNELS*sizeof(u4_t));
  Serial.print("LMIC.channelFreq: ");
  for (size_t i=0; i<MAX_CHANNELS; ++i) { Serial.println(LMIC.channelFreq[i], DEC); }
  Serial.println();
  // channelDlFreq
  memcpy(connState.channelDlFreq, LMIC.channelDlFreq, MAX_CHANNELS*sizeof(u4_t));
  Serial.print("LMIC.channelDlFreq: ");
  for (size_t i=0; i<MAX_CHANNELS; ++i) { Serial.println(LMIC.channelDlFreq[i], DEC); }
  Serial.println();
  // channelDrMap
  memcpy(connState.channelDrMap, LMIC.channelDrMap, MAX_CHANNELS*sizeof(u2_t));
  Serial.print("LMIC.channelDrMap: ");
  for (size_t i=0; i<MAX_CHANNELS; ++i) { Serial.println(LMIC.channelDrMap[i], DEC); }
  Serial.println();
  // channelMap
  connState.channelMap = LMIC.channelMap;
  Serial.println("LMIC and connState channelMap: ");
  Serial.println(LMIC.channelMap, BIN);
  Serial.println(connState.channelMap, BIN);
  // rx1DrOffset
  connState.rx1DrOffset = LMIC.rx1DrOffset;
  Serial.println("LMIC and connState rx1DrOffset: ");
  Serial.println(LMIC.rx1DrOffset, HEX);
  Serial.println(connState.rx1DrOffset, HEX);
  // rxDelay
  connState.rxDelay = LMIC.rxDelay;
  Serial.println("LMIC and connState rxDelay: ");
  Serial.println(LMIC.rxDelay, HEX);
  Serial.println(connState.rxDelay, HEX);
  // Write to Flash
  my_connection_store.write(connState);
  Serial.println(F("LMIC connection state successfully persisted."));
}


// Load persisted connection state and restore the connection.
// Returns false if there is no persisted state.
// See https://www.thethingsnetwork.org/forum/t/how-to-persist-lmic-otaa-parameters-with-an-esp32/35032/12
boolean restoreConnectionState() {
  // Temporary variable to hold the state loaded from Flash.
  LMIC_Connection_State connState;

  Serial.println(F("Load LMIC connection state from Flash ..."));
  // Read the content of "my_connection_store"
  connState = my_connection_store.read();
  if (connState.valid == false) {
    Serial.println(F("No valid connection state found..."));
    return false;
    }
  else {
    Serial.println(F("Restoring connection state..."));

    LMIC_setSession(connState.netid, connState.devaddr, connState.nwkKey, connState.artKey);
    // netid
    Serial.println("LMIC and connState netid: ");
    Serial.println(LMIC.netid, DEC);
    Serial.println(connState.netid, DEC);
    // devaddr
    Serial.println("LMIC and connState devaddr: ");
    Serial.println(LMIC.devaddr, HEX);
    Serial.println(connState.devaddr, HEX);
    // nwkKey
    Serial.print("LMIC.nwkKey: ");
    for (size_t i=0; i<sizeof(LMIC.nwkKey); ++i) {
            if (i != 0)
                    Serial.print("-");
            printHex2(LMIC.nwkKey[i]);
    }
    Serial.println();
    Serial.print("connState.nwkKey: ");
    for (size_t i=0; i<sizeof(connState.nwkKey); ++i) {
            if (i != 0)
                    Serial.print("-");
            printHex2(connState.nwkKey[i]);
    }
    Serial.println();        
    // artKey
    Serial.print("LMIC.artKey: ");
    for (size_t i=0; i<sizeof(LMIC.artKey); ++i) {
            if (i != 0)
                    Serial.print("-");
            printHex2(LMIC.artKey[i]);
    }
    Serial.println();
    Serial.print("connState.artKey: ");
    for (size_t i=0; i<sizeof(connState.artKey); ++i) {
            if (i != 0)
                    Serial.print("-");
            printHex2(connState.artKey[i]);
    }
    Serial.println();
    // seqnoUp -- restoring the seqNos is difficult, it does not work following an accidental reboot.
    LMIC_setSeqnoUp(connState.seqnoUp);
    Serial.println("LMIC and connState seqnoUp: ");
    Serial.println(LMIC.seqnoUp, DEC);
    Serial.println(connState.seqnoUp, DEC);
    // seqnoDn
    LMIC.seqnoDn = connState.seqnoDn;
    Serial.print("LMIC and connState seqnoDn: ");
    Serial.println(LMIC.seqnoDn, DEC);
    Serial.println(connState.seqnoDn, DEC);
    // dn2Dr
    LMIC.dn2Dr = connState.dn2Dr;
    Serial.println("LMIC and connState dn2Dr: ");
    Serial.println(LMIC.dn2Dr, DEC);
    Serial.println(connState.dn2Dr, DEC);
    // dnConf
    LMIC.dnConf = connState.dnConf;
    Serial.println("LMIC and connState dnConf: ");
    Serial.println(LMIC.dnConf, DEC);
    Serial.println(connState.dnConf, DEC);
    // txChnl
    LMIC.txChnl = connState.txChnl;
    Serial.println("LMIC and connState txChnl: ");
    Serial.println(LMIC.txChnl, DEC);
    Serial.println(connState.txChnl, DEC);    
    // datarate
    LMIC_setDrTxpow(connState.datarate, connState.adrTxPow);
    Serial.println("LMIC and connState datarate: ");
    Serial.println(LMIC.datarate, DEC);
    Serial.println(connState.datarate, DEC);
    // adrTxPow
    Serial.println("LMIC and connState adrTxPow: ");
    Serial.println(LMIC.adrTxPow, DEC);
    Serial.println(connState.adrTxPow, DEC);
    // adrAckReq
    LMIC.adrAckReq = connState.adrAckReq;
    Serial.print("LMIC and connState adrAckReq: ");
    Serial.println(LMIC.adrAckReq, HEX);
    Serial.println(connState.adrAckReq, HEX);
    // devNonc
    LMIC.devNonce = connState.devNonce;
    Serial.print("LMIC and connState devNonce: ");
    Serial.println(LMIC.devNonce, HEX);
    Serial.println(connState.devNonce, HEX);    
    // bands
    memcpy(LMIC.bands, connState.bands, MAX_BANDS*sizeof(band_t));
    // channelFreq
    memcpy(LMIC.channelFreq, connState.channelFreq, MAX_CHANNELS*sizeof(u4_t));
    Serial.print("LMIC.channelFreq: ");
    for (size_t i=0; i<MAX_CHANNELS; ++i) { Serial.println(LMIC.channelFreq[i], DEC); }
    Serial.println();
    // channelDlFreq
    memcpy(LMIC.channelDlFreq, connState.channelDlFreq, MAX_CHANNELS*sizeof(u4_t));
    Serial.print("LMIC.channelDlFreq: ");
    for (size_t i=0; i<MAX_CHANNELS; ++i) { Serial.println(LMIC.channelDlFreq[i], DEC); }
    Serial.println();
    // channelDrMap
    memcpy(LMIC.channelDrMap, connState.channelDrMap, MAX_CHANNELS*sizeof(u2_t));
    Serial.print("LMIC.channelDrMap: ");
    for (size_t i=0; i<MAX_CHANNELS; ++i) { Serial.println(LMIC.channelDrMap[i], DEC); }
    Serial.println();
    // channelMap
    LMIC.channelMap = connState.channelMap;
    Serial.println("LMIC and connState channelMap: ");
    Serial.println(LMIC.channelMap, BIN);
    Serial.println(connState.channelMap, BIN);    
    // rx1DrOffset
    LMIC.rx1DrOffset = connState.rx1DrOffset;
    Serial.println("LMIC and connState rx1DrOffset: ");
    Serial.println(LMIC.rx1DrOffset, HEX);
    Serial.println(connState.rx1DrOffset, HEX);    
    // rxDelay
    LMIC.rxDelay = connState.rxDelay;
    Serial.println("LMIC and connState rxDelay: ");
    Serial.println(LMIC.rxDelay, HEX);
    Serial.println(connState.rxDelay, HEX);

    // Overwrite channel plan and downlik RX2 according to EU-868 and TTN standard values
    setupEuChannels();
    return true;
  }
}

// Set up the channel scheme used in the EU-868 band plan.
// They are fixed; hence, there is no need to store and reload them
void setupEuChannels() {
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
    Serial.print("netid: ");
    Serial.println(netid, DEC);
    Serial.print("devaddr: ");
    Serial.println(devaddr, HEX);
    Serial.print("AppSKey: ");
    for (size_t i=0; i<sizeof(artKey); ++i) {
      if (i != 0)
        Serial.print("-");
      printHex2(artKey[i]);
    }
    Serial.println("");
    Serial.print("NwkSKey: ");
    for (size_t i=0; i<sizeof(nwkKey); ++i) {
            if (i != 0)
                    Serial.print("-");
            printHex2(nwkKey[i]);
    }
    Serial.println();            
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
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
            // Store the connection state to avoid joining in the future.
            persistConnectionState();
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
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
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
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
    delay(5000);
    while (! Serial)
        ;
    Serial.begin(9600);
    Serial.println(F("Starting"));
    Serial.print(F("Program Version "));
    Serial.println(progVer);

    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    if (restoreConnectionState() == true) {
      Serial.println(F("Restored connection from Flash, no JOIN neccessary."));
      printAddrAndKeys();
    }
    else {
      Serial.println(F("No valid stored connection found. Need to JOIN TTN."));
    }
  
    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}
