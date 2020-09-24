#include <lmic/lorabase.h>

//--------------------------------------------------------------------
//--  Keys for Mr. Black
//--------------------------------------------------------------------

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes.
static const u1_t PROGMEM DEVEUI[8] = { 0xE0, 0x93, 0x11, 0x00, 0x00, 0xB6, 0x76, 0x98 };

// This should also be in little endian format, see above.
// For TTN issued EUIs the last bytes should be 0xD5, 0xB3, 0x70.
static const u1_t PROGMEM APPEUI[8] = { 0xBA, 0x47, 0x03, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from the TTN console can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0x3E, 0x80, 0x01, 0x93, 0x8E, 0x2D, 0x42, 0x27, 0x97, 0x9C, 0xE3, 0x6E, 0x2A, 0x4F, 0x2C, 0x17 };

// LoRaWAN NwkSKey, network session key -- To directly connect without performing a JOIN
// This should be in big-endian (aka msb).
static const u1_t PROGMEM NWKSKEY[16] = { 0x04, 0xDC, 0x2E, 0x8A, 0xF8, 0x17, 0xB2, 0xA4, 0xBF, 0x30, 0x8E, 0x72, 0x07, 0x23, 0x0A, 0xD4 };

// LoRaWAN AppSKey, application session key -- To directly connect without performing a JOIN
// This should also be in big-endian (aka msb).
static const u1_t PROGMEM APPSKEY[16] = { 0x4D, 0x30, 0xF2, 0x8F, 0x17, 0xA6, 0x5F, 0x7C, 0xA6, 0x19, 0x76, 0x69, 0x16, 0x7B, 0xB1, 0x28 };

// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
// The library converts the address to network byte order as needed, so this should be in big-endian (aka msb) too.
static const u4_t DEVADDR = 0x26014C00;
