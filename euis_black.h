#include <lmic/lorabase.h>

// Define to resume an existing session without JOINING the TTN again.
// Requires DeviceAddr, NwSKey, and AppSKey to be defined below.
#ifndef CONN_RESUME
# define CONN_RESUME 1
#endif

//--------------------------------------------------------------------
//--  Keys
//--------------------------------------------------------------------
// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t APPEUI[8] = { 0xBA, 0x47, 0x03, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };

// This should also be in little endian format, see above.
static const u1_t DEVEUI[8] = { 0xBB, 0xDC, 0xA7, 0x62, 0xEA, 0xB7, 0x3C, 0x00 };

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from the TTN console can be copied as-is.
static const u1_t APPKEY[16] = { 0x13, 0x10, 0x26, 0xFD, 0x2F, 0x3D, 0x68, 0x44, 0x11, 0x49, 0x67, 0x90, 0x58, 0x81, 0xDB, 0x52 };

// LoRaWAN NwkSKey, network session key -- To directly connect without performing a JOIN
// This should be in big-endian (aka msb).
// Key for Mr. Black
static const u1_t NWKSKEY[16] = { 0x02, 0x55, 0xF0, 0x58, 0xB2, 0x2C, 0x63, 0xF5, 0x3B, 0xFE, 0x58, 0x03, 0xB9, 0x34, 0x9B, 0xDE };

// LoRaWAN AppSKey, application session key -- To directly connect without performing a JOIN
// This should also be in big-endian (aka msb).
// Key for Mr. Black
static const u1_t APPSKEY[16] = { 0xE7, 0x25, 0x7F, 0x43, 0x59, 0x94, 0xE0, 0x02, 0x4C, 0x00, 0x69, 0x3B, 0x6B, 0x85, 0x3D, 0x98 };

// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
// The library converts the address to network byte order as needed, so this should be in big-endian (aka msb) too.
static const u4_t DEVADDR = 0x26014730 ; // Address for Mr. Black
