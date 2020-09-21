#ifndef DEBUG_DISPLAY_H
#define DEBUG_DISPLAY_H

#include "display.h"

class DebugDisplay: public Display {
  public:
    void displayCurrentCO2Concentration(uint16_t co2Concentration) override;
};

#endif /* DEBUG_DISPLAY_H */
