#include <sys/socket.h>
#include <arpa/inet.h>  // For inet_addr()
#include <ifaddrs.h> // For extracting the ip address 
#include <cmath>
#include <algorithm>
#include "socket_stuff.hpp"
#include "dummy.hpp"

// IPv4 CHECKSUM calculation for a Packet.
// This function calculates the checksum over the entire packet.
unsigned short csum(const Packet &packet) {
    unsigned long sum = 0;
    const size_t size = packet.size();
    
    // Process 16-bit words.
    const uint16_t* ptr = reinterpret_cast<const uint16_t*>(packet.data());
    size_t nwords = size / 2;
    for (size_t i = 0; i < nwords; ++i) {
        sum += ptr[i];
    }
    
    // If there's an odd byte, pad with zero to form the last 16-bit word.
    if (size & 1) {
        sum += static_cast<uint8_t>(packet[size - 1]);
    }
    
    // Fold 32-bit sum to 16 bits: add carrier to result
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    // Return one's complement of the sum.
    return static_cast<unsigned short>(~sum);
}


// Getting the IP of the host
bool getLocalIP(sockaddr_in &localAddr) {
    struct ifaddrs *ifaddr, *ifa;
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return false;
    }
    
    bool found = false;
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;
        
        // Check for IPv4 addresses
        if (ifa->ifa_addr->sa_family == AF_INET) {
            // Optionally skip loopback interfaces (typically "lo")
            if (std::string(ifa->ifa_name) == "lo")
                continue;
            
            // Copy the IPv4 address into localAddr
            localAddr = *(reinterpret_cast<sockaddr_in*>(ifa->ifa_addr));
            found = true;
            break;  // Use the first found non-loopback IPv4 address
        }
    }
    
    freeifaddrs(ifaddr);
    return found;
}

// Building the packet in the correct form 
void float_to_int(float starting[], int quantized[] ){
    // FIND maximum absolute value
    float maximum = 0.0; 
    for (int i = 0; i < FLOAT_PER_PACKET; ++i){
        float absVal = std::fabs(starting[i]);
        if (absVal > maximum) {
            maximum = absVal;
        }
    }
    // IF max is 0 set 0 to everypacket 
    if (maximum == 0.0f) {
        for (int i = 0; i < FLOAT_PER_PACKET; ++i) {
            starting[i] = 0;
        }
        return; 
    }

    // Scaling factor 
    const int maxInt = (1 << (31 - 1)) - 1;  //32bit integer 
    float idealScale = static_cast<float>(maxInt) / maximum;

    // Force the scaling factor to be a power of two.
    int blockExponent = static_cast<int>(std::floor(std::log2(idealScale)));
    int commonScale = 1 << blockExponent;  // equivalent to 2^blockExponent

    for (int i = 0; i < FLOAT_PER_PACKET; ++i) {
        quantized[i] = static_cast<int>(std::round(starting[i] * commonScale));
    }

    return; 

}

void BuildPacket(Packet& packet, float * start, float * end){

    // Calculate required packet size.
    constexpr size_t headerSize = sizeof(struct iphdr) + sizeof(protocol_header);
    // Resize the vector to the exact packet size.
    packet.resize(headerSize);
    
    // Optionally, zero the packet if needed.
    std::memset(packet.data(), 0, headerSize);


    struct iphdr * ip = reinterpret_cast<struct iphdr*>(packet.data());
    protocol_header * new_hdr = reinterpret_cast<protocol_header*>(packet.data() + sizeof(struct iphdr));

    // Checking the float->int function 
    float arr[FLOAT_PER_PACKET]; 
    for (int i = 0; start<end; ++start, ++i){
        arr[i] = *start;   
    }
    int converted_array[FLOAT_PER_PACKET];
    float_to_int(arr, converted_array); 
    
    for (int i = 0; i < FLOAT_PER_PACKET; ++i){
        new_hdr->data[i] = htonl(converted_array[i]); 
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

    ip->check = csum(packet);//Computing the checksum

}

