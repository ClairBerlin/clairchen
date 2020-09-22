#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

typedef struct {
  uint16_t co2ppm;
  float temperature;
  float humidity;
} clair_sample_t;

class Sensor {
  public:
    /**
     * Returns false if setup fails, true otherwise.
     */
    virtual bool setup() = 0;

    /**
     * Sample measurements.
     */
    virtual clair_sample_t sampleMeasurements() = 0;

    /**
     * Return true if communication to the sensor failed.
     */
    virtual bool measurementFailed() = 0;
};

#endif /* SENSOR_H */
