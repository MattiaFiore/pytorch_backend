/*

*/



/*************************************************************************
 ***********************  H E A D E R S  *********************************
 *************************************************************************/


/*
REWRITING ATP HEADERS INTO NORMAL P4 
*/
const bit<16> TYPE_IPV4 = 0x0800; 
const bit<8> PROTOCOL_ML = 253; //FREE https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml

//const bit<16> NUMBER_POOLS = 5; 

typedef bit<48> ethAddr_t; 
typedef bit<32> ipAddr_t; 
typedef bit<16> udpPort_t; 


#define MAX_ENTRIES_PER_PACKET 25

// 14Bytes
header ethernet_t {
    ethAddr_t dstAddr; 
    ethAddr_t srcAddr; 
    bit<16> etherType;
}

// 20Bytes
header ipv4_t {
    bit<4> version; 
    bit<4> ihl; 
    bit<6> dscp; 
    bit<2> ecn; 
    bit<16> totalLen;
    bit<16> identification; 
    bit<3> flags; 
    bit<13> fragOffset; 
    bit<8> ttl; 
    bit<8> protocol; 
    bit<16> hdrChecksum; 
    ipAddr_t srcAddr; 
    ipAddr_t dstAddr; 
}


typedef bit<32> value_t; 
//100 Bytes 
header data_h{
    bit<32> pool_index; 
    value_t n01;
    value_t n02;
    value_t n03; 
    value_t n04;
    value_t n05;
    value_t n06;
    value_t n07;
    value_t n08;
    value_t n09;
    value_t n10;
    value_t n11;
    value_t n12;
    value_t n13; 
    value_t n14;
    value_t n15;
    value_t n16;
    value_t n17;
    value_t n18;
    value_t n19;
    value_t n20;
    value_t n21; 
    value_t n22; 
    value_t n23; 
    value_t n24; 
}



struct headers {
    ethernet_t eth; 
    ipv4_t ipv4; 
    data_h data; 
}

// METADATA 
struct metadata{
    bit<32> register_index; //This will be 0, 20, 40,  ...
    //bit<32> current_counter; 
}