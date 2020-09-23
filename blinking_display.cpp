#include "blinking_display.h"
#include <Arduino.h>
#include <arduino_lmic.h>

#define PERIOD_GOOD 1000
#define PERIOD_FAIR 500
#define PERIOD_BAD 250
#define PERIOD_ERROR 125
#define PERIOD_ERROR_SILENCE 1000

// unfortunately, LMIC's osjob_t does not support job-specific data
// so we have to keep this in static memory
static osjob_t blinkJob;
static uint16_t currentPeriod;
static int currentValue;
static int errorCodeNumber;
static int errorBlinkCounter;


static bool errorHasOccurred() {
  return errorCodeNumber != static_cast<int>(ErrorCode::NO_ERROR);
}

static void blink(osjob_t *job) {
  currentValue = currentValue == HIGH ? LOW : HIGH;
  digitalWrite(LED_BUILTIN, currentValue);

  if (errorHasOccurred()) {
    if (errorBlinkCounter < errorCodeNumber) {
      currentPeriod = PERIOD_ERROR;
      if (currentValue == HIGH) {
        errorBlinkCounter += 1;
      }
    } else {
      currentPeriod = PERIOD_ERROR_SILENCE;
      errorBlinkCounter = 0;
    }
  }

  ostime_t nextBlinkTick = os_getTime() + ms2osticks(currentPeriod);
  os_setTimedCallback(&blinkJob, nextBlinkTick, blink);
}


void BlinkingDisplay::setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  currentPeriod = PERIOD_GOOD;
  currentValue = HIGH;
  errorCodeNumber = static_cast<int>(ErrorCode::NO_ERROR);
  errorBlinkCounter = 0;
  os_setCallback(&blinkJob, blink);
}

void BlinkingDisplay::displayCurrentCO2Concentration(uint16_t co2Concentration) {
  DebugDisplay::displayCurrentCO2Concentration(co2Concentration);

  if (errorHasOccurred()) return;

  switch (concentrationToAirQuality(co2Concentration)) {
    case CO2AirQuality::veryGood:
    case CO2AirQuality::good:
      currentPeriod = PERIOD_GOOD;
      break;
    case CO2AirQuality::fair:
      currentPeriod = PERIOD_FAIR;
      break;
    default:
      currentPeriod = PERIOD_BAD;
  }
}

void BlinkingDisplay::displayError(ErrorCode errorCode) {
  DebugDisplay::displayError(errorCode);
  currentPeriod = PERIOD_ERROR;
  errorCodeNumber = static_cast<int>(errorCode);
}
