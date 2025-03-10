#include <iostream>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <vector>
#include <cstring>  // for memset
#include <cstdlib>  // for exit
#include <unistd.h> // for close
#include <string>

#define CUSTOM_PROTOCOL 253

// Define the custom protocol header with 25 float fields.
struct protocol_header {
    int pool_index; 
    int data[25]; // 25 floats -> 100 bytes
} __attribute__((packed));

// Packet alias: a dynamic-size byte buffer.
using Packet = std::vector<char>;

// IP checksum calculation.
unsigned short csum(const Packet &packet) {
    unsigned long sum = 0;
    const size_t size = packet.size();
    
    // Process 16-bit words.
    const uint16_t* ptr = reinterpret_cast<const uint16_t*>(packet.data());
    size_t nwords = size / 2;
    for (size_t i = 0; i < nwords; ++i) {
        sum += ptr[i];
    }
    
    // If there's an odd byte, pad with zero.
    if (size & 1) {
        sum += static_cast<uint8_t>(packet[size - 1]);
    }
    
    // Fold 32-bit sum to 16 bits.
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return static_cast<unsigned short>(~sum);
}

int main(int argc, char* argv[]){

    if (argc != 2){
        exit(EXIT_FAILURE); 
    }

    // Create a raw socket.
    int sock = socket(AF_INET, SOCK_RAW, CUSTOM_PROTOCOL);
    if (sock < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set the IP_HDRINCL option so we can provide our own IP header.
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    std::cout << "Everything is fine." << std::endl;

    // Calculate the total packet size: IP header + custom header.
    size_t total_size = sizeof(struct iphdr) + sizeof(protocol_header);
    
    // Resize the packet vector to hold the entire packet.
    Packet packet;
    packet.resize(total_size);
    
    // Zero the packet buffer.
    memset(packet.data(), 0, packet.size());

    // Get pointers to the IP header and our custom header within the buffer.
    struct iphdr* ip = reinterpret_cast<struct iphdr*>(packet.data());
    protocol_header* new_hdr = reinterpret_cast<protocol_header*>(packet.data() + sizeof(struct iphdr));
    
    // Fill in the custom header data (25 float values).
    new_hdr->pool_index = ntohl(std::stoi(argv[1])); 

    for (int i = 0; i < 25; ++i){
        new_hdr->data[i] = ntohl(1);   
    }

    // Fill in the IP header.
    ip->ihl      = 5;
    ip->version  = 4;
    ip->tos      = 0;
    ip->tot_len  = htons(total_size);
    ip->id       = htons(54321);  // Identification (arbitrary)
    ip->frag_off = 0;             // No fragmentation
    ip->ttl      = 64;
    ip->protocol = CUSTOM_PROTOCOL;
    ip->check    = 0;             // Checksum (set to 0 before calculation)
    ip->saddr    = inet_addr("10.0.0.2");  // Source IP address (adjust as needed)
    ip->daddr    = inet_addr("10.0.0.1");  // Destination IP address

    // Compute the IP checksum.
    ip->check = csum(packet);

    // Setup the destination address structure.
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = ip->daddr;

    // Send the packet.
    if (sendto(sock, packet.data(), packet.size(), 0,
               reinterpret_cast<struct sockaddr*>(&dest), sizeof(dest)) < 0){
        perror("sendto");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Packet sent." << std::endl;
    close(sock);
    return 0;
}
