# Next task that will be implemented 

- [ ] Create a queue to receive the packets 
- [ ] Fix receiving loop 
- [ ] Verify the correctness of receive through a python script 
- [ ] fix the wait after send finished (remove time.sleep(100))


# Important stuff that will be implemented after 

- [ ] Add the function to convert float into integers 
- [ ] Implementing the aggregation in the switch 
- [ ] Implement a timer for resending packets that were not acked 


# Completed tasks 
- [x] Each host must set his own IP address in an automatic whay
- [x] Sending the actual tensor data 
- [x] Create the mininet topology in order to execute the code to test it 
- [x] Added sending loop in a thread
- [x] Added queue to manage the packet that should be sent
- [x] Fix sending function 
- [x] Verify send for bigger tensors 

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