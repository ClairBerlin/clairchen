#include "debug-display.h"
#include "debug.h"

void DebugDisplay::displayCurrentCO2Concentration(uint16_t co2Concentration) {
  PRINT(F("CO2 concentration: "));
  switch (concentrationToAirQuality(co2Concentration)) {
    case CO2AirQuality::veryGood:
      PRINT(F("very good"));
      break;
    case CO2AirQuality::good:
      PRINT(F("good"));
      break;
    case CO2AirQuality::fair:
      PRINT(F("fair"));
      break;
    case CO2AirQuality::bad:
      PRINT(F("bad"));
      break;
    default:
      PRINT(F("critical"));
  }
  PRINT(F(" ("));
  PRINT(co2Concentration);
  PRINTLN(F(" ppm)"));
}
