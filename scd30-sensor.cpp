#include "scd30-sensor.h"
#include <Wire.h>
#include <Arduino.h>
#include <SparkFun_SCD30_Arduino_Library.h>

void Scd30Sensor::setup() {
  Wire.begin();
  scd30.begin();
}

clair_sample_t Scd30Sensor::sampleMeasurements() {
  clair_sample_t newSample;

  newSample.co2ppm = scd30.getCO2();
  newSample.temperature = scd30.getTemperature();
  newSample.humidity = scd30.getHumidity();

  return newSample;
}
