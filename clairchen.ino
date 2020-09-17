// We need to be able to compile with different options without editing source.
// When building with a more advanced environment, set the following variable:
// ARDUINO_LMIC_PROJECT_CONFIG_H=my_project_config.h
// otherwise the lmic_project_config.h from the LMIC library folder will be used
#ifndef ARDUINO_LMIC_PROJECT_CONFIG_H
# define ARDUINO_LMIC_PROJECT_CONFIG_H /home/jan/Code/ClAir/clairchen/project_config/lmic_project_config.h
#endif

#include <lmic.h>
#include "clair.h"
#include "things_network.h"

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

static osjob_t clairjob;
static Clair clair;
static int currentDatarate;

void setup() {
  PRINT_INIT();

  clair.setup();

#if CONN_RESUME
  resumeConnection();
#endif

  LMIC_reset();
  LMIC_setDrTxpow(DR_SF12, 14);
  currentDatarate = LMIC.datarate;
  
  os_init();

  // start first measurement
  measureAndSendIfDue(&clairjob);
}

void loop() {
  os_runloop_once();
}

static void scheduleNextMeasurement() {
  os_setTimedCallback(&clairjob, clair.getNextSampleTick(currentDatarate), measureAndSendIfDue);
}

static void measureAndSendIfDue(osjob_t* job) {
  clair.addSample();

  if (clair.isMessageDue(currentDatarate)) {
    uint8_t messageBuffer[CLAIR_MAX_MESSAGE_SIZE];
    uint8_t messageLength;
    lmic_tx_error_t error;

    messageLength = clair.encodeMessage(messageBuffer, CLAIR_MAX_MESSAGE_SIZE);

    error = LMIC_setTxData2(1, messageBuffer, messageLength, 0);
    if (error != 0) {
      // TODO
    }
    // next measurement will be scheduled on EV_TXCOMPLETE
  } else {
    scheduleNextMeasurement();
  }
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
      printAddrAndKeys();
      // Enable automatic data rate adjustment
      LMIC_setAdrMode(1);
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

      currentDatarate = LMIC.datarate;
      scheduleNextMeasurement();
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
