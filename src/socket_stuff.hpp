#ifndef SOCKET_STUFF_H_
#define SOCKET_STUFF_H_
#include <netinet/ip.h>
#include <vector>
#define CUSTOM_PROTOCOL 253

//CUSTOM PROTOCOL 
struct protocol_header {
  float data[25]; //100bytes of data 
}__attribute__((packed));

//Packet Alias 
using Packet = std::vector<char>; //Packet is dynamic-size 

//IP CHECKSUM CALCULATION 
unsigned short csum(const Packet & packet); 
// FUNCTION TO BUILD THE PACKET 
void BuildPacket(Packet& packet, float * start, float * end);
// FUNCTION TO EXTRACT AUTOMATICALLY THE IP FOR THE HOST
bool getLocalIP(struct sockaddr_in &source); 

#endif