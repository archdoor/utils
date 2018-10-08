#ifndef __CRC_H__
#define __CRC_H__


// 计算CRC校验码：多项式CRC-CCITT: 0x11021 X^16+X^12+X^5+1
unsigned short calc_crc(unsigned char *data, int len);
// 小端存储的CRC校验
int crc_check_little(unsigned char *data, int len, unsigned short *raw_crc);
// 大端存储的CRC校验
int crc_check_big(unsigned char *data, int len, unsigned short *raw_crc);







#endif
