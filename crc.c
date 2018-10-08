#include <stdio.h>
#include <arpa/inet.h>
#include "crc.h"

// 计算CRC校验码：多项式 0x11021 X^16+X^12+X^5+1
unsigned short calc_crc(unsigned char *data, int len)
{
    if( len <= 0 )
        return 0;

    unsigned short crc = 0;
    unsigned char byte;
    unsigned short byte_first_bit;
    unsigned short crc_first_bit;

    for( int i = 0; i < len; i++ )
    {
        byte = data[i];
        for( int j = 0; j < 8; j++ )
        {
            byte_first_bit = 0;
            crc_first_bit = 0;

            if ( byte & 0x80 ) {
                byte_first_bit = 1;
            }
            byte <<= 1;

            if ( crc & 0x8000 ) {
                crc_first_bit = 1;
            }
            crc <<= 1;

            if( byte_first_bit + crc_first_bit == 1 ) {
                crc ^= 0x1021;
            }
        }
    }
    return crc;
}

// CRC校验(校验码小端存储)
int crc_check_little(unsigned char *data, int len, unsigned short *raw_crc)
{
    unsigned short crc = calc_crc(data, len);
    if ( crc != *raw_crc )
    {
        return -1;
    }

    return 0;
}

// CRC校验(校验码大端存储)
int crc_check_big(unsigned char *data, int len, unsigned short *raw_crc)
{
    unsigned short crc = calc_crc(data, len);
    if ( htons(crc) != *raw_crc )
    {
        return -1;
    }

    return 0;
}


#ifdef DEBUG_CRC

int main()
{
    unsigned char buff [200] = {
    0xff, 0x7e, 0x6f, 0x00, 0x04, 0x12, 0x01, 0x00, 0x00, 0x00, 0x34, 0x36, 0x30, 0x31, 0x31, 0x33,
    0x33, 0x30, 0x32, 0x34, 0x34, 0x37, 0x31, 0x33, 0x34, 0x7d, 0xee, 0xca, 0x5a, 0xc1, 0x04, 0x06,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7d, 0xee, 0xca, 0x5a, 0x12, 0x01, 0xc4, 0xfa, 0xb5, 0xc4, 0xca,
    0xd6, 0xb3, 0xd6, 0xb6, 0xcb, 0xce, 0xc0, 0xd0, 0xc7, 0xb6, 0xa8, 0xce, 0xbb, 0xd0, 0xc5, 0xba,
    0xc5, 0xbd, 0xcf, 0xb2, 0xee, 0xa3, 0xac, 0xb0, 0xb2, 0xc8, 0xab, 0xb8, 0xe6, 0xbe, 0xaf, 0xb7,
    0xfe, 0xce, 0xf1, 0xbf, 0xc9, 0xc4, 0xdc, 0xca, 0xdc, 0xd3, 0xb0, 0xcf, 0xec, 0xa3, 0xac, 0xc7,
    0xeb, 0xd7, 0xa2, 0xd2, 0xe2, 0xd7, 0xf7, 0xd2, 0xb5, 0xb0, 0xb2, 0xc8, 0xab
    };

    unsigned short crc = calc_crc(buff, 109);
    printf("%02x\n", crc);

    unsigned char *tmp = (unsigned char *)&crc;
    printf("%02x\n", *tmp);
    printf("%02x\n", *(tmp+1));
}

#endif

