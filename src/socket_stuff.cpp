#include <sys/socket.h>
#include <arpa/inet.h>  // For inet_addr()

#include "socket_stuff.hpp"
#include "dummy.hpp"

// IPv4 CHECKSUM calculation 
unsigned short csum(unsigned short *buf, int nwords){    
    unsigned long sum = 0;
    for (; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

void BuildPacket(char (&packet)[4096]){

    struct iphdr * ip = (struct iphdr *) packet; 
    protocol_header * new_hdr = (protocol_header *) (packet + sizeof(struct iphdr)); 
    for (int i = 0; i < 25; ++i){
        new_hdr->data[i] = 0xCCCC; 
    }

    ip->ihl = 5; 
    ip->version = 4;
    ip->tos      = 0;                    // Type of Service
    ip->tot_len  = htons(sizeof(struct iphdr) + sizeof(protocol_header));
    ip->id       = htons(54321);          // Identification (arbitrary)
    ip->frag_off = 0;                    // No fragmentation
    ip->ttl      = 64;                   // Time to Live
    ip->protocol = CUSTOM_PROTOCOL;          // Custom protocol type (or use a specific protocol number)
    ip->check    = 0;                    // Checksum (initially 0 before calculation)
    ip->saddr    = inet_addr("10.0.0.1");   // Source IP address (adjust as needed)
    ip->daddr    = inet_addr("10.0.0.2");

    ip->check = csum((unsigned short *) packet, ip->ihl * 2);//Computing the checksum

}