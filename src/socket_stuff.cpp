#include <sys/socket.h>
#include <arpa/inet.h>  // For inet_addr()
#include <ifaddrs.h> // For extracting the ip address 
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
void BuildPacket(Packet& packet, float * start, float * end){

    // Calculate required packet size.
    constexpr size_t headerSize = sizeof(struct iphdr) + sizeof(protocol_header);
    // Resize the vector to the exact packet size.
    packet.resize(headerSize);
    
    // Optionally, zero the packet if needed.
    std::memset(packet.data(), 0, headerSize);

    struct iphdr * ip = reinterpret_cast<struct iphdr*>(packet.data());
    protocol_header * new_hdr = reinterpret_cast<protocol_header*>(packet.data() + sizeof(struct iphdr));
    for (int i = 0; start<end; ++start, ++i){
        new_hdr->data[i] = *start;   
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

