#include "protocolUtils.h"
#if defined(_WIN32) || defined(WIN32)  || defined(_WIN64) || defined(_WINDOWS)
#include <Windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif  // WIN32
#include <stdio.h>
#include <string.h>


namespace star_protocol
{

#define MAX_LENGTH_BYTE     2

int getTlvLength(const uint8_t *data, int &bytes)
{
    int length = 0;
    bytes = 0;
    do
    {
        length = length | ((data[bytes] & 0x7F) << (bytes * 7));
        if ((data[bytes] & 0x80) == 0)
        {
            break;
        }
    } while (++bytes < MAX_LENGTH_BYTE);
    bytes++;
    return length;
}

int setTlvLength(int length, uint8_t *buf, int bufSize, int &bytes)
{
    bytes = 0;
    do
    {
        if (bufSize < bytes + 1 || bytes > MAX_LENGTH_BYTE)
        {
            return -1;
        }
        buf[bytes] = length & 0x7F;
        length = length >> 7;
        if (length > 0)
        {
            buf[bytes] |= 0x80;
        }
        bytes++;
    } while (length > 0);
    return 0;
}

}