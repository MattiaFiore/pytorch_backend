#ifndef SOCKET_STUFF_H_
#define SOCKET_STUFF_H_
#include <netinet/ip.h>

#define CUSTOM_PROTOCOL 253

//CUSTOM PROTOCOL 
struct protocol_header {
  float data[25]; //100bytes of data 
}__attribute__((packed));

//IP CHECKSUM CALCULATION 
unsigned short csum(unsigned short *buf, int nwords); 
void BuildPacket(char (&packet)[4096], float * start, float * end);

#endif