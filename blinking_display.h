#ifndef BLINKING_DISPLAY_H
#define BLINKING_DISPLAY_H

#include "debug_display.h"
#include <stdint.h>

/**
 * WARNING: The state of the BlinkingDisplay is kept in static memory, since
 * LMIC's osjob_t does not support job-specific data.
 * Make sure that only one BlinkingDisplay is instantiated.
 */
class BlinkingDisplay: public DebugDisplay {
  public:
    void setup() override;
    void displayCurrentCO2Concentration(uint16_t co2Concentration) override;
};

#endif /* BLINKING_DISPLAY_H */
