#include "clair.h"
#include "sensor.h"
#include <Arduino.h>
#include "debug.h"

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

#define NROF_TRANSMISSION_CONFIGS (sizeof(transmission_configs) / sizeof(transmission_configs[0]))

Clair::Clair(Sensor *sensorArg) {
  sensor = sensorArg;
  currentDatarate = 0; // SF12
  secondsSinceLastSample = 0;
  numberOfSamplesInBuffer = 0;
}

void Clair::setup() {
  sensor->setup();
}

uint16_t Clair::getCO2Concentration() {
  clair_sample_t sample;
  sample = sensor->sampleMeasurements();

  PRINT(F("sample: "));
  PRINT(F("CO2: "));
  PRINT(sample.co2ppm);
  PRINT(F(" ppm, temperature: "));
  PRINT(sample.temperature);
  PRINT(F(" °C, humidity: "));
  PRINT(sample.humidity);
  PRINTLN(F(" %"));

  secondsSinceLastSample += CLAIR_MEASURING_PERIOD_SECS;
  if (secondsSinceLastSample >= transmission_configs[currentDatarate].samplingPeriodMinutes * 60) {
    PRINTLN(F("adding sample to message buffer"));
    if (numberOfSamplesInBuffer == transmission_configs[currentDatarate].samplesPerMessage) {
      PRINTLN(F("message overdue, discarding oldest sample"));
      for (int i = 0; i < numberOfSamplesInBuffer - 1; i++) {
        sampleBuffer[i] = sampleBuffer[i + 1];
      }
      numberOfSamplesInBuffer -= 1;
    }
    sampleBuffer[numberOfSamplesInBuffer] = sample;
    numberOfSamplesInBuffer += 1;
    PRINT(F("number of samples in buffer: "));
    PRINTLN(numberOfSamplesInBuffer);

    secondsSinceLastSample = 0;
  }

  return sample.co2ppm;
}

void Clair::setCurrentDatarate(int datarate) {
  currentDatarate = datarate;

  PRINT(F("current datarate: "));
  PRINT_DATARATE(currentDatarate);
  PRINTLN("");

  transmission_config_t currentTransmissionConfig = transmission_configs[currentDatarate];
  PRINT(F("current sampling period [min]: ")); PRINTLN(currentTransmissionConfig.samplingPeriodMinutes);
  PRINT(F("current # of samples in message: ")); PRINTLN(currentTransmissionConfig.samplesPerMessage);
}

bool Clair::isMessageDue() {
  if (currentDatarate < 0 || currentDatarate > NROF_TRANSMISSION_CONFIGS) return false;
  return numberOfSamplesInBuffer >= transmission_configs[currentDatarate].samplesPerMessage;
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
  if (!isMessageDue()) return 0;

  // encode header
  uint8_t *bufferPosition = messageBuffer;
  *bufferPosition = 0;
  *bufferPosition |= CLAIR_PROTOCOL_VERSION << 5; // protocol version 1 (3 bits)
  *bufferPosition |= CLAIR_MESSAGE_ID_SAMPLE_LIST << 3; // message identifier (2 bits)
  *bufferPosition |= transmission_configs[currentDatarate].samplesPerMessage - 1; // message header (3 bits), # of bytes - 1!!!
  bufferPosition += 1;

  uint8_t messageLength = 1;

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
