#include "clair.h"
#include <arduino_lmic.h>

typedef struct {
  uint8_t samplingPeriodMinutes;
  uint8_t samplesPerMessage;
} transmission_config_t;

// the order of this array corresponds to the LMIC DR_XX enums
static transmission_config_t transmission_configs[] = {
  { .samplingPeriodMinutes = 14, .samplesPerMessage = 5 }, // SF12+
  { .samplingPeriodMinutes = 9, .samplesPerMessage = 4 }, // SF11+
  { .samplingPeriodMinutes = 3, .samplesPerMessage = 5 }, // SF10+
  { .samplingPeriodMinutes = 3, .samplesPerMessage = 3 }, // SF9+
  { .samplingPeriodMinutes = 2, .samplesPerMessage = 2 }, // SF8
  { .samplingPeriodMinutes = 1, .samplesPerMessage = 2 }, // SF7
  { .samplingPeriodMinutes = 1, .samplesPerMessage = 2 } // SF7/B
};

Clair::Clair() {
  numberOfSamplesInBuffer = 0;
}

void Clair::setup() {
  scd30.begin();
}

void Clair::addSample() {
  clair_sample_t newSample;

  if (! numberOfSamplesInBuffer < CLAIR_MAX_NROF_SAMPLES_PER_MESSAGE) return;

  tickLastSample = os_getTime();

  newSample.co2ppm = scd30.getCO2();
  newSample.temperature = scd30.getTemperature();
  newSample.humidity = scd30.getHumidity();

  sampleBuffer[numberOfSamplesInBuffer] = newSample;
  numberOfSamplesInBuffer += 1;
}

bool Clair::isMessageDue(int datarate) {
  currentDatarate = datarate;
  return numberOfSamplesInBuffer >= transmission_configs[datarate].samplesPerMessage;
}

ostime_t Clair::getNextSampleTick(int datarate) {
  uint32_t currentSamplingPeriodMinutes = transmission_configs[datarate].samplingPeriodMinutes;
  return tickLastSample + (ostime_t) (1000 * 60 * currentSamplingPeriodMinutes);
}

static void encodeSample(clair_sample_t sample, uint8_t *messageBuffer) {
  messageBuffer[0] = (sample.co2ppm + 10) / 20;

  messageBuffer[1] = 0;
  messageBuffer[1] |= (((uint8_t) (sample.temperature + 0.5)) & 0x1F) << 3; // 5 bits
  messageBuffer[1] |= ((uint8_t) (sample.humidity + 5) / 10) & 0x7; // 3 bits
}

#define CLAIR_PROTOCOL_VERSION 0
#define CLAIR_MESSAGE_ID_SAMPLE_LIST 0

uint8_t Clair::encodeMessage(uint8_t *messageBuffer, uint16_t messageBufferSize) {
  uint8_t messageLength = 1;
  uint8_t *bufferPosition = messageBuffer;

  // encode header
  *bufferPosition = 0;
  *bufferPosition |= CLAIR_PROTOCOL_VERSION << 5; // protocol version 1 (3 bits)
  *bufferPosition |= CLAIR_MESSAGE_ID_SAMPLE_LIST << 3; // message identifier (2 bits)
  *bufferPosition |= transmission_configs[currentDatarate].samplesPerMessage - 1; // message header (3 bits)

  // encode samples
  for (int i = 0; i < numberOfSamplesInBuffer; i++) {
    encodeSample(sampleBuffer[i], bufferPosition);
    bufferPosition += 2;
    messageLength += 2;
  }

  // reset sample buffer
  numberOfSamplesInBuffer = 0;

  return messageLength;
}
