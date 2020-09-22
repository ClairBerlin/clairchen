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
    /**
     * Constructor
     */
    Clair(Sensor *sensor);

    /**
     * To be called in the Arduino setup hook
     *
     * If an I2C/TWI sensor is used, Wire.begin() must be called before.
     *
     * Returns false if setup fails, true otherwise.
     */
    bool setup();

    /**
     * Get CO2 concentration in ppm
     *
     * Must be called once per CLAIR_MEASURING_PERIOD_SECS.
     *
     * Returns a negative value if the measurement failed.
     */
    int16_t getCO2Concentration();

    /**
     * Set the current datarate which determines the transmission rate.
     *
     * Should only be called after the transmission of a message.
     */
    void setCurrentDatarate(int datarate);

    /**
     * Returns whether a message is due
     *
     * Should be called after calling getCO2Concentration().
     * If a message is due, encodeMessage() should be called and the message be sent.
     * If encodeMessage() is not called even though a message is due the oldest 
     * sample is discarded once a new sample is added to the message buffer.
     */
    bool isMessageDue();

    /**
     * Returns the length of the encoded message
     */
    uint8_t encodeMessage(uint8_t *messageBuffer, uint16_t messageBufferSize);

  private:
    Sensor *sensor;

    clair_sample_t minuteBuffer[60 / CLAIR_MEASURING_PERIOD_SECS];
    uint8_t indexOfNextSampleInMinuteBuffer;

    void addSampleToMinuteBuffer(clair_sample_t sample);
    clair_sample_t getAverageSampleOfLastMinute();

    clair_sample_t sampleBuffer[CLAIR_MAX_NROF_SAMPLES_PER_MESSAGE];
    uint16_t secondsSinceLastSample;
    uint8_t numberOfSamplesInBuffer;

    int currentDatarate;
};

#endif /* CLAIR_H */
