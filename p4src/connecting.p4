#include <core.p4>
#include <v1model.p4>

#define ML_PROTO 253 

typedef bit<48> ethAddr_t; 
typedef bit<32> ipAddr_t; 


header ethernet_t {
    ethAddr_t dstAddr; 
    ethAddr_t srcAddr; 
    bit<16> etherType;
}

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

header ml_proto_t{
    bit<8> queue_id; 
    bit<24> queue_occupancy; 
}
struct headers {
    ethernet_t eth; 
    ipv4_t ipv4; 
    ml_proto_t ml; 
}

struct metadata{}

parser MyParser(packet_in packet,
                out headers hdr,
                inout metadata meta,
                inout standard_metadata_t standard_metadata) {

                

    state start {
        transition parse_eth; 
    }
    state parse_eth{
        packet.extract(hdr.eth);
        transition select(hdr.eth.etherType){
            0x0800: parse_ipv4; 
            default: accept; 
        }
    }

    state parse_ipv4{
        packet.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol){
            ML_PROTO: parse_ml_proto; 
            default: accept; 
        }
    }

    state parse_ml_proto{
        packet.extract(hdr.ml); 
        transition accept; 
    }

}
/*************************************************************************
************   C H E C K S U M    V E R I F I C A T I O N   *************
*************************************************************************/

control MyVerifyChecksum(inout headers hdr, inout metadata meta) {
    apply {}
}

/*************************************************************************
**************  I N G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

control MyIngress(inout headers hdr,
                  inout metadata meta,
                  inout standard_metadata_t standard_metadata) {

    action send(bit<8> output){
        standard_metadata.egress_spec = (bit<9>) output; 
    }

    apply{

        if (hdr.ipv4.dstAddr == 0x0A000001){
            standard_metadata.egress_spec = (bit<9>) 1; 
        } else if (hdr.ipv4.dstAddr == 0x0A000002) {
            standard_metadata.egress_spec = (bit<9>) 2; 
        } else {
            standard_metadata.egress_spec = (bit<9>) 3;
        }
    }
    
}

/*************************************************************************
****************  E G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

control MyEgress(inout headers hdr,
                 inout metadata meta,
                 inout standard_metadata_t standard_metadata) {

    apply {

        if (hdr.ml.isValid()){
            hdr.ml.queue_occupancy = (bit<24>) standard_metadata.enq_qdepth; 
        }
    }

}

/*************************************************************************
*************   C H E C K S U M    C O M P U T A T I O N   **************
*************************************************************************/

control MyComputeChecksum(inout headers hdr, inout metadata meta) {
apply{}
}



control MyDeparser(packet_out packet, in headers hdr) {
    apply {
        packet.emit(hdr.eth);
        packet.emit(hdr.ipv4);
        packet.emit(hdr.ml); 
    }
}


/*************************************************************************
***********************  S W I T C H  *******************************
*************************************************************************/

//switch architecture
V1Switch(
MyParser(),
MyVerifyChecksum(),
MyIngress(),
MyEgress(),
MyComputeChecksum(),
MyDeparser()
) main;