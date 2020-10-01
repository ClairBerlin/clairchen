#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// we're testing static functions
#include "../clair.cpp"

#define NROF_ELEMENTS_OF(ARY) (sizeof(ARY) / sizeof(ARY[0]))

TEST_CASE("Temperature bytes are encoded correctly", "[clair]") {
  struct ExpectedEncodings {
    float temperature;
    uint8_t byte;
  };

  struct ExpectedEncodings expectedEncodings[] = {
    { .temperature = -5.0, .byte = 0 },
    { .temperature = 0.0, .byte = 0 },
    { .temperature = 5.0, .byte = 5 },
    { .temperature = 5.5, .byte = 6 },
    { .temperature = 12.6, .byte = 13 },
    { .temperature = 35.0, .byte = 31 }
  };

  for (unsigned int i = 0; i < NROF_ELEMENTS_OF(expectedEncodings); i++) {
    REQUIRE(encodeTemperatureByte(expectedEncodings[i].temperature) == expectedEncodings[i].byte);
  }
}

TEST_CASE("Humidity bytes are encoded correctly", "[clair]") {
  struct ExpectedEncodings {
    float humidity;
    uint8_t byte;
  };

  struct ExpectedEncodings expectedEncodings[] = {
    { .humidity = 0.0, .byte = 0 },
    { .humidity = 5.0, .byte = 0 },
    { .humidity = 10.0, .byte = 0 },
    { .humidity = 14.0, .byte = 0 },
    { .humidity = 15.0, .byte = 1 },
    { .humidity = 20.0, .byte = 1 },
    { .humidity = 24.0, .byte = 1 },
    { .humidity = 25.0, .byte = 2 },
    { .humidity = 79.0, .byte = 7 },
    { .humidity = 80.0, .byte = 7 },
    { .humidity = 85.0, .byte = 7 },
    { .humidity = 100.0, .byte = 7 }
  };

  for (unsigned int i = 0; i < NROF_ELEMENTS_OF(expectedEncodings); i++) {
    REQUIRE(encodeHumidityByte(expectedEncodings[i].humidity) == expectedEncodings[i].byte);
  }
}

TEST_CASE("CO2 values are encoded correctly", "[clair]") {
  struct ExpectedEncodings {
    uint16_t co2ppm;
    uint8_t byte;
  };

  struct ExpectedEncodings expectedEncodings[] = {
    {.co2ppm = 0, .byte = 0 },
    {.co2ppm = 10, .byte = 1 },
    {.co2ppm = 15, .byte = 1 },
    {.co2ppm = 20, .byte = 1 },
    {.co2ppm = 400, .byte = 20 },
    {.co2ppm = 415, .byte = 21 },
    {.co2ppm = 2000, .byte = 100 },
    {.co2ppm = 5100, .byte = 255 },
    {.co2ppm = 5500, .byte = 255 }
  };

  for (unsigned int i = 0; i < NROF_ELEMENTS_OF(expectedEncodings); i++) {
    REQUIRE(encodeCO2ppmByte(expectedEncodings[i].co2ppm) == expectedEncodings[i].byte);
  }
}
