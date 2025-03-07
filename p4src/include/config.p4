
const int register_size = 100; 

// Change this 
const int n_workers = 2; 

// Registers where the aggregation will happen 
register<value_t>(register_size) pool; 
// Counter register 
register<bit<8>>(register_size) counter_pool; 