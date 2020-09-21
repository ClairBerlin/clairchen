#ifndef CLAIR_H
#define CLAIR_H

#include <stdint.h>
#include "sensor.h"

/* sizes in bytes */
#define CLAIR_HEADER_SIZE 1
#define CLAIR_SAMPLE_SIZE 2
#define CLAIR_MAX_NROF_SAMPLES_PER_MESSAGE 5
#define CLAIR_MAX_MESSAGE_SIZE (CLAIR_MAX_NROF_SAMPLES_PER_MESSAGE * CLAIR_SAMPLE_SIZE + CLAIR_HEADER_SIZE)

#define CLAIR_MEASURING_PERIOD_SECS 5
#define CLAIR_MAX_SAMPLINE_PERIOD_MINS 14

/**
 * A Clair object keeps the state of the transmission adaptation algorithm.
 */
class Clair {
  public:
    Clair(Sensor *sensor);

    /**
     * To be called in the Arduino setup hook.
     */
    void setup();

    /**
     * Get CO2 concentration in ppm.
     *
     * Must be called once per CLAIR_MEASURING_PERIOD_SECS
     */
    uint16_t getCO2Concentration();

    /**
     * Set the current datarate which determines the transmission rate.
     *
     * Should only be called after the transmission of a message.
     */
    void setCurrentDatarate(int datarate);

    bool isMessageDue();

    /**
     * Returns the length of the encoded message
     */
    uint8_t encodeMessage(uint8_t *messageBuffer, uint16_t messageBufferSize);

  private:
    Sensor *sensor;
    clair_sample_t sampleBuffer[CLAIR_MAX_NROF_SAMPLES_PER_MESSAGE];
    uint16_t secondsSinceLastSample;
    uint8_t numberOfSamplesInBuffer;
    int currentDatarate;
};

#endif /* CLAIR_H */
