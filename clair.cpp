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
  indexOfNextSampleInMinuteBuffer = 0;
}

bool Clair::setup() {
  return sensor->setup();
}

#define NROF_SAMPLES_IN_MINUTE_BUFFER (sizeof(minuteBuffer) / sizeof(minuteBuffer[0]))

void Clair::addSampleToMinuteBuffer(clair_sample_t sample) {
  minuteBuffer[indexOfNextSampleInMinuteBuffer] = sample;
  indexOfNextSampleInMinuteBuffer = (indexOfNextSampleInMinuteBuffer + 1) % NROF_SAMPLES_IN_MINUTE_BUFFER;
}

clair_sample_t Clair::getAverageSampleOfLastMinute() {
  uint32_t sumOfCo2ppms = 0;
  float sumOfTemperatures = 0;
  float sumOfHumidities = 0;

  for (uint16_t i = 0; i < NROF_SAMPLES_IN_MINUTE_BUFFER; i++) {
    sumOfCo2ppms += minuteBuffer[i].co2ppm;
    sumOfTemperatures += minuteBuffer[i].temperature;
    sumOfHumidities += minuteBuffer[i].humidity;
  }

  clair_sample_t averageSample;
  averageSample.co2ppm = (sumOfCo2ppms + (NROF_SAMPLES_IN_MINUTE_BUFFER / 2)) / NROF_SAMPLES_IN_MINUTE_BUFFER;
  averageSample.temperature = sumOfTemperatures / NROF_SAMPLES_IN_MINUTE_BUFFER;
  averageSample.humidity = sumOfHumidities / NROF_SAMPLES_IN_MINUTE_BUFFER;

  return averageSample;
}

#define PRINT_SAMPLE(SAMPLE) do { \
    PRINT(F("CO2: ")); \
    PRINT(SAMPLE.co2ppm); \
    PRINT(F(" ppm, temperature: ")); \
    PRINT(SAMPLE.temperature); \
    PRINT(F(" °C, humidity: ")); \
    PRINT(SAMPLE.humidity); \
    PRINTLN(F(" %")); \
  } while (0)

int16_t Clair::getCO2Concentration() {
  if (sensor->measurementFailed()) return -1;

  clair_sample_t sample = sensor->sampleMeasurements();

  PRINT(F("sample: "));
  PRINT_SAMPLE(sample);

  addSampleToMinuteBuffer(sample);
  secondsSinceLastSample += CLAIR_MEASURING_PERIOD_SECS;

  if (secondsSinceLastSample >= transmission_configs[currentDatarate].samplingPeriodMinutes * 60) {
    PRINTLN(F("adding average sample of last minute to message buffer"));

    if (numberOfSamplesInBuffer == transmission_configs[currentDatarate].samplesPerMessage) {
      PRINTLN(F("message overdue, discarding oldest sample"));
      for (int i = 0; i < numberOfSamplesInBuffer - 1; i++) {
        sampleBuffer[i] = sampleBuffer[i + 1];
      }
      numberOfSamplesInBuffer -= 1;
    }

    clair_sample_t averageSample = getAverageSampleOfLastMinute();
    PRINT(F("average sample of last minute: "));
    PRINT_SAMPLE(averageSample);

    sampleBuffer[numberOfSamplesInBuffer] = averageSample;
    numberOfSamplesInBuffer += 1;
    PRINT(F("number of samples in buffer: "));
    PRINTLN(numberOfSamplesInBuffer);

    secondsSinceLastSample = 0;
  }

  return static_cast<int16_t>(sample.co2ppm);
}

void Clair::setCurrentDatarate(int datarate) {
  if (datarate < 0 || datarate > (int) NROF_TRANSMISSION_CONFIGS) {
    PRINT(F("WARNING: invalid datarate: ")); PRINTLN(datarate);
    datarate = 0;
  }

  currentDatarate = datarate;

  PRINT(F("current datarate: "));
  PRINT_DATARATE(currentDatarate);
  PRINTLN("");
  PRINT(F("current sampling period [min]: ")); PRINTLN(transmission_configs[currentDatarate].samplingPeriodMinutes);
  PRINT(F("current # of samples in message: ")); PRINTLN(transmission_configs[currentDatarate].samplesPerMessage);
}

bool Clair::isMessageDue() {
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
  if (messageBufferSize < 1) return 0;

  // encode header
  uint8_t *bufferPosition = messageBuffer;
  *bufferPosition = 0;
  *bufferPosition |= CLAIR_PROTOCOL_VERSION << 6; // protocol version (2 bits)
  *bufferPosition |= CLAIR_MESSAGE_ID_SAMPLE_LIST << 3; // message identifier (3 bits)
  // NOTE: to use all bits of the message header to full capacity we transmit the number of samples - 1!
  *bufferPosition |= transmission_configs[currentDatarate].samplesPerMessage - 1; // message header (3 bits)
  bufferPosition += 1;

  uint8_t messageLength = 1;

  // encode samples
  for (int i = 0; i < numberOfSamplesInBuffer; i++) {
    messageLength += 2;
    if (messageLength > messageBufferSize) {
      PRINT(F("WARNING: message buffer size too small:"));
      PRINTLN(messageBufferSize);
      return 0;
    }
    encodeSample(sampleBuffer[i], bufferPosition);
    bufferPosition += 2;
  }

  // reset sample buffer
  numberOfSamplesInBuffer = 0;

  return messageLength;
}
