#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define DISPLAY_THRESHOLD_GOOD      450
#define DISPLAY_THRESHOLD_FAIR      700
#define DISPLAY_THRESHOLD_BAD       1000
#define DISPLAY_THRESHOLD_CRITICAL  2500

enum class CO2AirQuality { veryGood, good, fair, bad, critical };

class Display {
  public:
    virtual void setup() { };

    virtual void displayCurrentCO2Concentration(uint16_t co2Concentration) = 0;

    inline CO2AirQuality concentrationToAirQuality(uint16_t co2Concentration) {
      if (co2Concentration < DISPLAY_THRESHOLD_GOOD) return CO2AirQuality::veryGood;
      if (co2Concentration < DISPLAY_THRESHOLD_FAIR) return CO2AirQuality::good;
      if (co2Concentration < DISPLAY_THRESHOLD_BAD) return CO2AirQuality::fair;
      if (co2Concentration < DISPLAY_THRESHOLD_CRITICAL) return CO2AirQuality::bad;
      return CO2AirQuality::critical;
    }
};

#endif /* DISPLAY_H */
