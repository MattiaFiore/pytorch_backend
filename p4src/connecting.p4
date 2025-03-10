#include <core.p4>
#include <v1model.p4>
#include "include/headers.p4"
#include "include/parser.p4"
#include "include/config.p4"


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

     
    // Takes in input a number and the pool index
    // it will sum that number to the register at specified index
    // It will also increase the counter of the 
    action sum(in value_t n0,
            in bit<32> index){
        
        // Sum number 
        bit<32> value;  
        pool.read(value, index);
        value = value + n0; 
        pool.write(index, value);
        // Increase counter 

        bit<8> value_counter; 
        counter_pool.read(value_counter, index);
        value_counter = value_counter + 1; 
        counter_pool.write(index, value_counter);
        //meta.current_counter = value; 
    }

    action retrieve_index(bit<32> register_index) {

        meta.register_index = register_index; 
            
    }

    action send(bit<8> output){
        standard_metadata.egress_spec = (bit<9>) output; 
    }

    action aggregate_values(){
        sum(hdr.data.n01, meta.register_index);
        sum(hdr.data.n02, meta.register_index + 1);
        sum(hdr.data.n03, meta.register_index + 2);
        sum(hdr.data.n04, meta.register_index + 3);
        sum(hdr.data.n04, meta.register_index + 4);
        sum(hdr.data.n04, meta.register_index + 5);
        sum(hdr.data.n04, meta.register_index + 6);
        sum(hdr.data.n04, meta.register_index + 7);
        sum(hdr.data.n04, meta.register_index + 8);
        sum(hdr.data.n04, meta.register_index + 9);
        sum(hdr.data.n04, meta.register_index + 10);
        sum(hdr.data.n04, meta.register_index + 11);
        sum(hdr.data.n04, meta.register_index + 12);
        sum(hdr.data.n04, meta.register_index + 13);
        sum(hdr.data.n04, meta.register_index + 14);
        sum(hdr.data.n04, meta.register_index + 15);
        sum(hdr.data.n04, meta.register_index + 16);
        sum(hdr.data.n04, meta.register_index + 17);
        sum(hdr.data.n04, meta.register_index + 18);
        sum(hdr.data.n04, meta.register_index + 19);
        sum(hdr.data.n04, meta.register_index + 20);
        sum(hdr.data.n04, meta.register_index + 21);
        sum(hdr.data.n04, meta.register_index + 22);
        sum(hdr.data.n04, meta.register_index + 23);
        sum(hdr.data.n04, meta.register_index + 24);
    }
    
    
    table pool_access{
        key = {hdr.data.pool_index: exact;}
        actions = {
            retrieve_index; 
            NoAction;
        }
        size = 4; // Same dimension as the number of pools, right now is 5
        default_action = NoAction; 
    }
    

    // FIX THIS
    apply{

        if (hdr.ipv4.dstAddr == 0x0A000001){
            standard_metadata.egress_spec = (bit<9>) 1; 
        } else if (hdr.ipv4.dstAddr == 0x0A000002) {
            standard_metadata.egress_spec = (bit<9>) 2; 
        } else {
            standard_metadata.egress_spec = (bit<9>) 3;
        }
        // this function will put in the metadata.register_index the index scaled  
        if (hdr.ipv4.protocol == PROTOCOL_ML){
            pool_access.apply();
            aggregate_values(); 
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
        packet.emit(hdr.data); 
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