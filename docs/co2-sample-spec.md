# Sample Specification for ClAirchen Message #0

The ClAirchen prototype Node uses a [Sensirion SCP30](https://www.sensirion.com/en/environmental-sensors/carbon-dioxide-sensors/carbon-dioxide-sensors-co2/) sensor module for CO&#x2082; temperature, and relative humidity. We encode all three values into one sample of two bytes as follows:

## CO&#x2082; Measurement

The SCP30 sensor measures CO&#x2082; concentration in parts per million (PPM), over a range from 0 to 40,000PPM. Fresh air has a concentration about 450PPM. According to DIN EN 16798-1, a concentration of 2,000PPM is considered critical for the design of ventilation systems, and deemed unacceptable as a measure for SARS-COV-2 concentration (see [Table 1](https://blogs.tu-berlin.de/hri_sars-cov-2/wp-content/uploads/sites/154/2020/08/hartmann_kriegel_2020_de_v3.pdf)). The maximum admissible workplace concentration is 5,000 PPM; e.g., in mines.

The SCP30 has an accuracy of +/-30PPM. We limit the range of values to (0PPM, 5,100PPM) and quantize it at 20PPM, for a total of 255 values. These values are encoded in the high-byte of a sample in big-endian encoding (MSB first).

Example: the bit sequence (msb..lsb) 0110 0100 (0x64) stands for 2000 PPM.

## Temperature Measurement

The SCP30 includes a temperature sensor to compensate its CO&#x2082; measurement. For us, the temperature reading is interesting in its own right; e.g., to perform plausibility checks on the CO&#x2082; measurement, to determine if windows are opened, or to detect if the Node has been moved. The sensor covers a range from -40° to +70° at +/-0.4° accuracy. For realistic indoor temperatures, we limit the measurement range to 0° to 31° and encode the value with 5 bits in big-endian format (MSB first) as part of the low-byte of the sample.

## Humidity

The SCP30 includes a humidity sensor, which provides values of percentage relative humidity in the range from 0% to 100%. Humidity might provide an additional indication about the number of people in a room or the physical activity performed there. We limit the range to realistic 10% to 80%, quantize the values in 10% increments and encode the resulting value in 3 bits (000b for 10%, 111b for 80%) in big-endian encoding, as part of the low-byte of the sample.

## Sample Format

Taken together, one ClAirchen sample has size 2 bytes and is structured as follows:

| CO&#x2082; (1 byte) | temp. (5 bit); rel. hum. (3 bit) |
