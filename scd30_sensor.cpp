#include "scd30_sensor.h"
#include <Wire.h>
#include <Arduino.h>
#include <SparkFun_SCD30_Arduino_Library.h>

Scd30Sensor::Scd30Sensor() {
  error = false;
}

bool Scd30Sensor::setup() {
  bool result = scd30.begin();
  // make sure SCD30 has data available
  delay(2000);
  return result;
}

clair_sample_t Scd30Sensor::sampleMeasurements() {
  // we're assuming that sampleMeasurements isn't called more often than once
  // per 2 s, i.e., data should always be available
  if (!scd30.dataAvailable()) error = true;

  // the SCD30 library always returns the latest measurement from its cache
  clair_sample_t newSample;
  newSample.co2ppm = scd30.getCO2();
  newSample.temperature = scd30.getTemperature();
  newSample.humidity = scd30.getHumidity();
  return newSample;
}

bool Scd30Sensor::measurementFailed() {
  return error;
}
