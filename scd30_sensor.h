#ifndef SCD30_SENSOR_H
#define SCD30_SENSOR_H

#include "sensor.h"
#include <SparkFun_SCD30_Arduino_Library.h>

class Scd30Sensor: public Sensor {
  public:
    Scd30Sensor();

    /**
     * Wire.begin() must be called before calling Scd30Sensor::setup()
     */
    bool setup() override;

    clair_sample_t sampleMeasurements() override;

    bool measurementFailed() override;
    
  private:
    SCD30 scd30;
    int errorCounter;
};

#endif /* SCD30_SENSOR_H */
