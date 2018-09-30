#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ljpcap.h"

void timeval_to_string(struct timeval *time, char *sdate)
{
    char buff[64] = {0};
    struct tm dc = {0};
    localtime_r(&time->tv_sec, &dc);
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", &dc);
    sprintf(sdate, "%s.%.6ld", buff, time->tv_usec);
}

void memprint(uint8_t *data, uint32_t datalen)
{
    for ( int i = 0; i < datalen; ++i )
    {
        printf("%02x ", data[i]);
        if ( ((i + 1) % 16) == 0 )
        {
            printf("\n");
        }
    }
    printf("\n");
}

void print_pcap_file_header(pcap_file_header_t *file_header)
{
    printf("-------------pcap_file_header-------------\n");
    printf("[magic]:%#x\n", file_header->magic);
    printf("[version_major]:%u\n", file_header->version_major);
    printf("[version_minor]:%u\n", file_header->version_minor);
    printf("[thiszone]:%u\n", file_header->thiszone);
    printf("[sigfigs]:%u\n", file_header->sigfigs);
    printf("[snaplen]:%u\n", file_header->snaplen);
    printf("[linktype]:%u\n\n", file_header->linktype);
}
void print_pcap_header(pcap_header_t *pcap_header)
{
    struct timeval time = { pcap_header->time_sec, pcap_header->time_usec };
    char sdate[64] = {0};
    timeval_to_string(&time, sdate);
    printf("-------------pcap_header-------------\n");
    printf("[time]:%s\n", sdate);
    printf("[caplen]:%u\n", pcap_header->caplen);
    printf("[len]:%u\n", pcap_header->len);
}
void print_ethernet_header(ethernet_header_t *ethernet_header)
{
    printf("-------------ethernet_header-------------\n");
    printf("[dst_mac]: %02x:%02x:%02x:%02x:%02x:%02x\n", ethernet_header->dst_mac[0], ethernet_header->dst_mac[1], ethernet_header->dst_mac[2], ethernet_header->dst_mac[3], ethernet_header->dst_mac[4], ethernet_header->dst_mac[5]);
    printf("[src_mac]: %02x:%02x:%02x:%02x:%02x:%02x\n", ethernet_header->src_mac[0], ethernet_header->src_mac[1], ethernet_header->src_mac[2], ethernet_header->src_mac[3], ethernet_header->src_mac[4], ethernet_header->src_mac[5]);
    printf("[type]: %u\n", ethernet_header->type);
}

void print_ip_header(ip_header_t *ip_header)
{
    static struct in_addr saddr = {0};
    static struct in_addr daddr = {0};
    saddr.s_addr = ip_header->src_ip;
    daddr.s_addr = ip_header->dst_ip;

    printf("-------------ip_header-------------\n");
    printf("[version]: %u\n", ip_header->version);
    printf("[headerlen]: %u\n", ip_header->headerlen);
    printf("[tos]: %u\n", ip_header->tos);
    printf("[totallen]: %u\n", ip_header->totallen);
    printf("[id]: %u\n", ip_header->id);
    printf("[fragment_offset]: %#x\n", ip_header->fragment_offset);
    printf("[ttl]: %u\n", ip_header->ttl);
    printf("[protocol]: %u\n", ip_header->protocol);
    printf("[checksum]: %#x\n", ip_header->checksum);
    printf("[src_ip]: %s\n", inet_ntoa(saddr));
    printf("[dst_ip]: %s\n", inet_ntoa(daddr));
}

void print_udp_header(udp_header_t *udp_header)
{
    printf("-------------udp_header-------------\n");
    printf("[src_port]: %u\n", udp_header->src_port);
    printf("[dst_port]: %u\n", udp_header->dst_port);
    printf("[length]: %u\n", udp_header->length);
    printf("[checksum]: %#x\n", udp_header->checksum);
}

void print_user_data(uint8_t *data, uint32_t datalen)
{
    printf("-------------user_data-------------\n");
    memprint(data, datalen);
}

pcap_file_t *pcap_file_open(const char *path)
{
    pcap_file_t *file = malloc(sizeof(pcap_file_t));
    if ( file == NULL )
    {
        return NULL;
    }

    file->fp = fopen(path, "r");
    if ( file->fp == NULL )
    {
        free(file);
        return NULL;
    }

    if ( fread(&file->pcap_file_header, sizeof(pcap_file_header_t), 1, file->fp) != 1 )
    {
        fclose(file->fp);
        free(file);
        return NULL;
    }

    return file;
}

int get_pcap_package(pcap_file_t *file, pcap_package_t *package)
{
    if ( fread(&package->pcap_header, sizeof(pcap_header_t), 1, file->fp) != 1 )
    {
        return -1;
    }

    if ( fread(&package->ethernet_header, sizeof(ethernet_header_t), 1, file->fp) != 1 )
    {
        return -1;
    }

    if ( fread(&package->ip_header, sizeof(ip_header_t), 1, file->fp) != 1 )
    {
        return -1;
    }
    package->ip_header.totallen = ntohs(package->ip_header.totallen);
    package->ip_header.id = ntohs(package->ip_header.id);
    package->ip_header.fragment_offset = ntohs(package->ip_header.fragment_offset);
    package->ip_header.checksum = ntohs(package->ip_header.checksum);

    package->datalen = package->ip_header.totallen - sizeof(ip_header_t);
    if ( 0 )
    {
        if ( fread(&package->tcp_header, sizeof(tcp_header_t), 1, file->fp) != 1 )
        {
            return -1;
        }
        package->datalen -= sizeof(tcp_header_t);
    }
    else if ( package->ip_header.protocol == 0x11 )
    {
        if ( fread(&package->udp_header, sizeof(udp_header_t), 1, file->fp) != 1 )
        {
            return -1;
        }
        package->udp_header.src_port = ntohs(package->udp_header.src_port);
        package->udp_header.dst_port = ntohs(package->udp_header.dst_port); package->udp_header.length = ntohs(package->udp_header.length);
        package->udp_header.checksum = ntohs(package->udp_header.checksum);

        package->datalen -= sizeof(udp_header_t);
    }

    if ( package->datalen > LJPCAP_USER_DATA_MAX_LEN )
    {
        return -1;
    }

    if ( fread(package->data, package->datalen, 1, file->fp) != 1 )
    {
        return -1;
    }

    return 0;
}

void pcap_file_close(pcap_file_t *file)
{
    fclose(file->fp);
    free(file);
}

void print_pcap_package(pcap_package_t *package)
{
    print_pcap_header(&package->pcap_header);
    print_ethernet_header(&package->ethernet_header);
    print_ip_header(&package->ip_header);
    print_udp_header(&package->udp_header);
    print_user_data(package->data, package->datalen);
}

void print_pcap_file_info(pcap_file_t *file)
{
    print_pcap_file_header(&file->pcap_file_header);
}


#ifdef LJPCAP_DEBUG

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "ljpcap.h"

const char *pcap_file = "my.pcap";

int main(void)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd == -1 ) 
    {
        printf("new socket error!\n");
        return -1;
    }

    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_port = htons(31001),
        .sin_addr.s_addr = inet_addr("192.168.0.120"),
    };

    while ( 1 )
    {
        pcap_package_t package = {0};
        pcap_file_t *file = pcap_file_open(pcap_file);
        if ( file == NULL )
        {
            printf("open pcap file error!\n");
            return -1;
        }
        print_pcap_file_info(file);

        while ( get_pcap_package(file, &package) == 0 )
        {
            print_pcap_package(&package);

            if ( sendto(sockfd, package.data, package.datalen, 0, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in)) < 0)
            {
                printf("send data error!\n");
            }
            usleep(500);
        }

        pcap_file_close(file);
        break;
    }

    close(sockfd);
    return 0;
}

#endif



