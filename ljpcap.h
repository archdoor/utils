#ifndef __LIUJIE_PCAP_FILE_H__
#define __LIUJIE_PCAP_FILE_H__ 

#include <stdio.h>
#include <stdint.h>

#define LJPCAP_USER_DATA_MAX_LEN 2048

typedef struct _pcap_file_header
{
    uint32_t magic;
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t linktype;
} __attribute__ ((packed)) pcap_file_header_t;

typedef struct _pcap_header
{
    uint32_t time_sec;
    uint32_t time_usec;
    uint32_t caplen;
    uint32_t len;
} __attribute__ ((packed)) pcap_header_t;

typedef struct _ethernet_header
{
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t type;
} __attribute__ ((packed)) ethernet_header_t;

typedef struct _ip_header
{
    uint8_t headerlen:4;
    uint8_t version:4;
    uint8_t tos;
    uint16_t totallen;
    uint16_t id;
    uint16_t fragment_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
} __attribute__ ((packed)) ip_header_t;

typedef struct _tcp_header
{
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seqno;
    uint32_t ackno;
    uint8_t headerlen;
    uint8_t flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgentpointer;
} __attribute__ ((packed)) tcp_header_t;

typedef struct _udp_header
{
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__ ((packed)) udp_header_t;

typedef struct _pcap_file
{
    FILE *fp;
    pcap_file_header_t pcap_file_header;
}pcap_file_t;

typedef struct _pcap_package
{
    pcap_header_t pcap_header;
    ethernet_header_t ethernet_header;
    ip_header_t ip_header;
    union{
        tcp_header_t tcp_header;
        udp_header_t udp_header;
    };
    unsigned char data[LJPCAP_USER_DATA_MAX_LEN];
    uint32_t datalen;
}pcap_package_t;


pcap_file_t *pcap_file_open(const char *path);
int get_pcap_package(pcap_file_t *file, pcap_package_t *package);
void pcap_file_close(pcap_file_t *file);

void print_pcap_package(pcap_package_t *package);
void print_pcap_file_info(pcap_file_t *file);


#endif



