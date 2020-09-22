#ifndef SCD30_SENSOR_H
#define SCD30_SENSOR_H

#include "sensor.h"
#include <SparkFun_SCD30_Arduino_Library.h>

class Scd30Sensor: public Sensor {
  public:
    /**
     * Wire.begin() must be called before calling Scd30Sensor::setup()
     */
    void setup() override;

    clair_sample_t sampleMeasurements() override;
    
  private:
    SCD30 scd30;
};

#endif /* SCD30_SENSOR_H */
