// We need to be able to compile with different options without editing source.
// When building with a more advanced environment, set the following variable:
// ARDUINO_LMIC_PROJECT_CONFIG_H=my_project_config.h
// otherwise the lmic_project_config.h from the LMIC library folder will be used
#ifndef ARDUINO_LMIC_PROJECT_CONFIG_H
# define ARDUINO_LMIC_PROJECT_CONFIG_H /home/jan/Code/ClAir/clairchen/lmic_config.h
#endif

#include <arduino_lmic.h>
#include "scd30_sensor.h"
#include "clair.h"
#include "debug_display.h"
#include "things_network.h"
#include "debug.h"

static osjob_t clairjob;
static Scd30Sensor sensor;
static Clair clair(&sensor);
static DebugDisplay display;
static int currentDatarate;
static ostime_t tickLastSample;
static bool joined;

void setup() {
  PRINT_INIT();

  clair.setup();  

  os_init();

  LMIC_reset();

#if 1
  resumeConnection();
  joined = true;
#else
  LMIC_startJoining();
  joined = false;
#endif

  LMIC_setDrTxpow(DR_SF8, 14);
  clair.setCurrentDatarate(LMIC.datarate);
  
  os_setCallback(&clairjob, measureAndSendIfDue); 
}

void loop() {
  os_runloop_once();
}

static void measureAndSendIfDue(osjob_t* job) {
  uint16_t currentCO2Concentration = clair.getCO2Concentration();
  display.displayCurrentCO2Concentration(currentCO2Concentration);

  if (joined && clair.isMessageDue()) {
    uint8_t messageBuffer[CLAIR_MAX_MESSAGE_SIZE];
    uint8_t messageLength;
    lmic_tx_error_t error;

    PRINTLN("encoding message");

    messageLength = clair.encodeMessage(messageBuffer, CLAIR_MAX_MESSAGE_SIZE);

    error = LMIC_setTxData2(1, messageBuffer, messageLength, 0);
    if (error != 0) {
      // TODO
    }
  }

  os_setTimedCallback(&clairjob, os_getTime() + ms2osticks(1000L * CLAIR_MEASURING_PERIOD_SECS), measureAndSendIfDue);
}

void onEvent (ev_t ev) {
  PRINT(os_getTime());
  PRINT(": ");
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      PRINTLN(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      PRINTLN(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      PRINTLN(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      PRINTLN(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      PRINTLN(F("EV_JOINING"));
      break;
    case EV_JOINED:
      PRINTLN(F("EV_JOINED"));
      PRINT_ADDRESSES_AND_KEYS();
      // Enable automatic data rate adjustment
      LMIC_setAdrMode(1);
      clair.setCurrentDatarate(LMIC.datarate);
      joined = true;
      break;
    /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_RFU1:
      ||     PRINTLN(F("EV_RFU1"));
      ||     break;
    */
    case EV_JOIN_FAILED:
      PRINTLN(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      PRINTLN(F("EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      PRINTLN(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      // Any data to be received?
      if (LMIC.dataLen != 0 || LMIC.dataBeg != 0) {
        PRINT_RECEIVED_MSG();
        // Call user-supplied function with port #, pMessage, nMessage; // nMessage might be zero.
        // receiveMessage(bPort, LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
      } else {
        PRINTLN(F("  Did not receive any downlink message."));
      }
      PRINT("  Uplink Sequence Counter: ");
      PRINTLN(LMIC.seqnoUp);
      PRINT("  Downlink Sequence Counter: ");
      PRINTLN(LMIC.seqnoDn);
      
      clair.setCurrentDatarate(LMIC.datarate);
      break;
    case EV_LOST_TSYNC:
      PRINTLN(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      PRINTLN(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      PRINTLN(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      PRINTLN(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      PRINTLN(F("EV_LINK_ALIVE"));
      break;
    /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_SCAN_FOUND:
      ||    PRINTLN(F("EV_SCAN_FOUND"));
      ||    break;
    */
    case EV_TXSTART:
      PRINTLN(F("EV_TXSTART"));
      PRINT(F("channel: ")); PRINT(LMIC.txChnl);
      PRINT(F(", datarate: ")); PRINTLN(LMIC.datarate);
      break;
    case EV_TXCANCELED:
      PRINTLN(F("EV_TXCANCELED"));
      break;
    case EV_RXSTART:
      /* do not print anything -- it wrecks timing */
      break;
    case EV_JOIN_TXCOMPLETE:
      PRINTLN(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
      break;

    default:
      PRINT(F("Unknown event: "));
      PRINTLN((unsigned) ev);
      break;
  }

  PRINT_FLUSH();
}
