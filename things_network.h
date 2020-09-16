#ifndef THINGS_NETWORK_H
#define THINGS_NETWORK_H

void setupEu868Channels();
void resumeConnection();

#ifndef NDEBUG

void printAddrAndKeys();

#define TTN_PRINT_ADDR_AND_KEYS() do { \
  printAddrAndKeys(); \
} while (0)

#else /* NDEBUG */

#define TTN_PRINT_ADDR_AND_KEYS() do {} while (0)

#endif /* NDEBUG */

#endif THINGS_NETWORK_H
