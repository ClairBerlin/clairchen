#ifndef CLAIR_H
#define CLAIR_H

#include <stdint.h>
#include <Wire.h>
#include <SparkFun_SCD30_Arduino_Library.h>
#include <arduino_lmic.h>

#define CLAIR_HEADER_SIZE 1
#define CLAIR_SAMPLE_SIZE 2
#define CLAIR_MAX_NROF_SAMPLES_PER_MESSAGE 5
#define CLAIR_MAX_MESSAGE_SIZE (CLAIR_MAX_NROF_SAMPLES_PER_MESSAGE * CLAIR_SAMPLE_SIZE + CLAIR_HEADER_SIZE)

typedef struct {
  uint16_t co2ppm;
  float temperature;
  float humidity;
} clair_sample_t;

/**
 * A Clair object keeps the state of the transmission adaptation algorithm.
 */
class Clair {
  public:
    Clair();
    void setup();
    void addSample();
    bool isMessageDue(int datarate);
    /**
     * Returns the length of the encoded message
     */
    uint8_t encodeMessage(uint8_t *messageBuffer, uint16_t messageBufferSize);
    ostime_t getNextSampleTick(int datarate);

  private:
    SCD30 scd30;
    clair_sample_t sampleBuffer[CLAIR_MAX_NROF_SAMPLES_PER_MESSAGE];
    uint8_t numberOfSamplesInBuffer;
    ostime_t tickLastSample;
    int currentDatarate;
};

#endif /* CLAIR_H */
