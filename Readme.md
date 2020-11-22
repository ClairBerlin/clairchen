
# Readme

This repository contains the embedded software for our _Clairchen_ node, a simple do-it-yourself (DIY) CO2 sensor with connectivity to the [Clair Platform](https://github.com/ClairBerlin) via [The Things Network](https://www.thethingsnetwork.org) (TTN), a community-driven LoRaWAN network.

## Overview

Clairchen is an [Arduino](https://www.arduino.cc) application for the [Adafruit Feather M0 with RFM95 LoRa Radio Module](https://www.adafruit.com/product/3178) and the [Sensirion SCD 30](https://www.sensirion.com/en/environmental-sensors/carbon-dioxide-sensors/carbon-dioxide-sensors-co2/) CO2 sensor module.

The primary focus of Clairchen is to explore CO2 sensing and TTN data transmission.

## Features

- Measurement of CO2 concentration, temperature, and relative humidity.
- Display of the measured CO2 concentration in three levels via the Feather's on-board LED.
- Creation and transmission of a [payload-optimized packet](/docs/co2-sample-spec.md) of measurement samples to a configurable TTN application.

Because of the [TTN Airtime Constraint](https://www.thethingsnetwork.org/docs/lorawan/limitations.html) of 30s transmission per 24h, the amount of measurement samples that any CO2 sensor on TTN can report to the Clair Platform is severely restricted. For _Clairchen_, we developed an [optimized payload format](/docs/message-format.md) and [transmission scheme](/docs/sampling-and-transmission-scheme.md) that trades of the _sampling rate_: how often a Clairchen node performes a measurement - with the _transmission interval_: how often the node reports its measurements to the Clair platform. Especially for the robust low-rate modulation and coding schemes (MCS) that are used in conditions of bad radio coverage, we assemple multiple samples in one packet to minimize the LoRa protocol overhead.

## Limitations

Because Clairchen is a platform for exploration, we did not optimize it for low energy consumption. In particular, we do not use any sleep-mode features, and we read the sensor quite often to average of its readings. This leads to a very high power drain. Therefore, a DIY node for actual use as a measurement device either needs to run off an external USB power supply or you need to add power cycling code, reduce sensor readout, and more aggressively adapt power the transmission power.

## Design

The _Clairchen Node_ uses the [Adafruit Feather M0 with RFM95 LoRa Radio](https://www.adafruit.com/product/3178) and the [Sensirion SCD 30](https://www.sensirion.com/en/environmental-sensors/carbon-dioxide-sensors/carbon-dioxide-sensors-co2/) CO2 sensor module.

We designed the Clairchen software around the [MCCI LoRaWAN LMIC Library](https://github.com/mcci-catena/arduino-lmic), which provides the main event loop, creates the timing reference, and interfaces with the LoRa radio module.

In addition to the Arduino SAMD Board-Support Package (BSP) and the Adafruit SAMD Board Package for Arduino, we use the following libraries:

- [MCCI LoRaWAN LMIC Library](https://github.com/mcci-catena/arduino-lmic) (MIT License)
- [SparkFun SCD30 Arduino Library](https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library) (MIT License)
- [Arduino Wire Library](https://www.arduino.cc/en/Reference/Wire) to flash LED.

## Development Setup

Prepare your Adafruit Feather M0 as shown in the [Adafruit tutorial](https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/overview). Then wire up the SCD30 sensor as an I2C-device. The pinout is as follows:

```c
// Pin mapping for Adafruit Feather M0 LoRa, etc.
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {3, 6, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 8,              // LBT cal for the Adafruit Feather M0 LoRa, in dB
    // D-stepping of the silicon, so the 10Mhz SPI clock should work.
};
```

Next, place [LMIC project configuration file](/project_config/lmic_project_config.h) into the project config folder of the LMIC library; typically, this is in your Arduino path at `<arduino>/libraries/MCCI_LoRaWAN_LMIC_library/project_config`.

Connect the Feather via USB to your host, load the present repository into your Arduino IDE, and upload the resulting sketch to the uC.

You can test your setup with your own TTN application as described [here](https://learn.adafruit.com/the-things-network-for-feather) and [here](https://blog.werktag.io/posts/adafruit-feather-m0-lora-on-ttn/)
