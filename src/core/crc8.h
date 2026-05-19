#ifndef CRC8_H
#define CRC8_H

#include <stdint.h>
#include <string.h>

// CRC-8-CCIT, generator polynomial: x**8 + x**2 + x**1 + x**0
unsigned char crc8(const void * data, int size);

#endif // CRC8_H
