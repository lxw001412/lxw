#ifndef _CRC_CRC32_H
#define _CRC_CRC32_H

#include <stdint.h>

namespace star_protocol
{

uint32_t crc32(const uint8_t* s, int len);
uint32_t crc32(const uint8_t* s1, int len1, const uint8_t* s2, int len2);

}

#endif