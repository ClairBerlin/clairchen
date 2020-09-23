#include <lmic/lorabase.h>

//--------------------------------------------------------------------
//--  Keys for Mr. Black
//--------------------------------------------------------------------

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes.
static const u1_t PROGMEM DEVEUI[8] = { 0xBB, 0xDC, 0xA7, 0x62, 0xEA, 0xB7, 0x3C, 0x00 };

// This should also be in little endian format, see above.
// For TTN issued EUIs the last bytes should be 0xD5, 0xB3, 0x70.
static const u1_t PROGMEM APPEUI[8] = { 0xBA, 0x47, 0x03, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from the TTN console can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0x13, 0x10, 0x26, 0xFD, 0x2F, 0x3D, 0x68, 0x44, 0x11, 0x49, 0x67, 0x90, 0x58, 0x81, 0xDB, 0x52 };

// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
// The library converts the address to network byte order as needed, so this should be in big-endian (aka msb) too.
static const u4_t DEVADDR = 0x2601233E;

// LoRaWAN NwkSKey, network session key -- To directly connect without performing a JOIN
// This should be in big-endian (aka msb).
static const u1_t PROGMEM NWKSKEY[16] = { 0x4D, 0x40, 0xA8, 0x7B, 0x0C, 0x63, 0xE6, 0x1A, 0x78, 0xE5, 0xF3, 0xFF, 0x7B, 0x4C, 0xC2, 0x10 };

// LoRaWAN AppSKey, application session key -- To directly connect without performing a JOIN
// This should also be in big-endian (aka msb).
static const u1_t PROGMEM APPSKEY[16] = { 0x88, 0x9C, 0xEB, 0xD1, 0xB7, 0xA9, 0xF8, 0xDC, 0x3B, 0xA5, 0xED, 0xA9, 0x8D, 0xD6, 0x92, 0x1A };
