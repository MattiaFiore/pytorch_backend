# Next task that will be implemented 

*BACKEND*: 

- [ ] Fix return type of the recv function (???)
- [ ] Remove unused variables 
- [ ] Add the function to convert integers into float 
- [ ] Add the mutex to access `in_flight_packets` 
- [ ] On the receive access `in_flight_packets` and reduce it


*SWITCH*: 
- [ ] modify parsing with new header
- [ ] Remove hardcoded routing
- [ ] Assign pool indexes to different jobs 

*TEST SCRIPT*


*CONTROLLER*

*PROTOCOL*
- [ ] Fix better bounds for the fields 
- [ ] Check for missing fields

# Important stuff that will be implemented after 

- [ ] Implement a timer for resending packets that were not acked 
- [ ] Sender/Receiver should do a handshake to agree on the size of the tensor that their are going to exchange (this should already be solved since receive depends on the size of the tensor given in input)
- [ ] Add controller to tell how many register are available in the switch 
- [ ] Add a way to filter out packets that are not needed in the receivingLoop 
- [ ] Use the prints only if specified in the execution in order to work only when debugging

# Completed tasks 
*BACKEND* 
- [x] Each host must set his own IP address in an automatic whay
- [x] Sending the actual tensor data 
- [x] Create the mininet topology in order to execute the code to test it 
- [x] Added sending loop in a thread
- [x] Added queue to manage the packet that should be sent
- [x] Fix sending function 
- [x] Verify send for bigger tensors 
- [x] Fix receiving loop 
- [x] Create a queue to receive the packets 
- [x] Add the recv function
- [x] fix the wait after send finished (remove time.sleep(100))
- [x] Verify the correctness of receive through a python script 
- [x] Set maximum sending queue size = number of registers assigned 
- [x] Modify the protocol in order to add the rest of the information
- [x] Add the function to convert float into integers 

*SWITCH*
- [x] Add parser 
- [x] Add registers to do the aggregation 

*TEST SCRIPT*
- [x] Add the pool index that can be inserted by terminal command 
- [x] Modify the code in order to use int values 

*CONTROLLER*
- [x] Add a script to see the values inside the registers in order to debug 


# Running the code

Start the network 
```bash 
sudo python3 network/network.py
```
Start the code for the test in the host 
```bash 
sudo python3 test_scripts/one_of_the_tests 
```
This should be run in order to make the library visible for the linker 
```bash 
echo "/usr/local/lib/python3.8/dist-packages/torch/lib" | sudo tee /etc/ld.so.conf.d/torch.conf
sudo ldconfig

```