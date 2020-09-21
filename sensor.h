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
    virtual void setup() = 0;
    virtual clair_sample_t sampleMeasurements() = 0;
};

#endif /* SENSOR_H */
