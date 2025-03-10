#ifndef SOCKET_STUFF_H_
#define SOCKET_STUFF_H_
#include <netinet/ip.h>
#include <vector>
#include <iostream>

#define CUSTOM_PROTOCOL 253
#define FLOAT_PER_PACKET 25

//CUSTOM PROTOCOL 
struct protocol_header {
  char jobID; // 8bits -> 256 maximum number of concurrent job 
  long int sequence_number; // 32bits -> 4 miliardi? 
  short int counter; // 16 bits -> 65536 maximum number of computes
  char isAck; // Change this should be just one bit
  int data[25]; //100bytes of data 
}__attribute__((packed));

//Packet Alias 
using Packet = std::vector<char>; //Packet is dynamic-size 
// float->int
void float_to_int(float starting[], int quantized[]);
//IP CHECKSUM CALCULATION 
unsigned short csum(const Packet & packet); 

// FUNCTION TO BUILD THE PACKET 
void BuildPacket(Packet& packet, float * start, float * end);
// FUNCTION TO EXTRACT AUTOMATICALLY THE IP FOR THE HOST
bool getLocalIP(struct sockaddr_in &source); 

#endif