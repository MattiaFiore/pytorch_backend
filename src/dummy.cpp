#include <iostream>
#include <pybind11/pybind11.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <pybind11/stl.h>
#include <torch/extension.h>

// MY libraries 
#include "dummy.hpp"
#include "socket_stuff.hpp"

// Tell pybind11 to use c10::intrusive_ptr as the holder type:
//PYBIND11_DECLARE_HOLDER_TYPE(T, c10::intrusive_ptr<T>)

namespace py = pybind11;

namespace c10d {

using std::cout; 

BackendDummy::BackendDummy(int rank, int size)
    : Backend(rank, size) {

  
  std::cout << "Constructing BackendDummy with rank=" << rank 
            << " and size=" << size << std::endl;
  
  // In the constructor the socket is initialized. 
  cout<< "Starting the socket..." <<std::endl; 

  // Getting the IP address 
  if (!getLocalIP(IPV4_addr)){
        std::cout << "ERRORE" << std::endl; 
        exit(EXIT_FAILURE); 
  }

  inet_ntop(AF_INET, &IPV4_addr.sin_addr, ipstring, sizeof(ipstring));
  std::cout << "The IP address is: " << ipstring << std::endl; 

  // Starting the sending socket 
  sockFD_send_ = socket(AF_INET, SOCK_RAW, CUSTOM_PROTOCOL); 
  if (sockFD_send_ < 0){
    cout << "ERRORE" << std::endl; \
    perror("socket"); 
    exit(EXIT_FAILURE); 
  }
  
  int one = 1;
    if (setsockopt(sockFD_send_, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
  cout << "Started the SEND socket" << std::endl; 

  // Starting the receiving  socket 
  sockFD_recv_ = socket(AF_INET, SOCK_RAW, CUSTOM_PROTOCOL); 
  if (sockFD_recv_ < 0){
    cout << "ERRORE" << std::endl; 
    perror("socket"); 
    exit(EXIT_FAILURE); 
  }
  
  if (setsockopt(sockFD_recv_, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  
  cout << "Started the RECV socket" << std::endl; 
  
  sendThread_ = std::thread(&BackendDummy::sendingLoop, this); 
  recvThread_ = std::thread(&BackendDummy::receivingLoop, this);
   

}

void BackendDummy::sendingLoop() {
  while (!stopThreads_) { 
    Packet packet;
      struct sockaddr_in dest;
      memset(&dest, 0, sizeof(dest));
      dest.sin_family = AF_INET;
      dest.sin_addr.s_addr = inet_addr("10.0.0.2");

    {
      //Lock the queeue mutex 
      std::unique_lock<std::mutex> lock(send_queueMutex_);
      // wait for the queue to be not empty or for the threads to be stopped 
      send_queueCV_.wait(lock, [&]{ return !sendQueue_.empty() || stopThreads_; });
        if (stopThreads_) {
          break;
        } else {
          packet = sendQueue_.front();
          
          sendQueue_.pop();
        }
    }
    
    if (sendto(sockFD_send_, packet.data(), packet.size(), 0,
         (struct sockaddr *)&dest, sizeof(dest)) < 0) { 
      perror("sendto");
      //close(sockFD_recv_);
      //close(sockFD_send_); 
      exit(EXIT_FAILURE); 

    
    }
    cout << "[SENDING LOOP] Pacchetto inviato " << std::endl; 
    
    // Optionally, implement waiting for the ack here, possibly with another condition variable or timeout.
  }
}

void BackendDummy::receivingLoop() {
  
  Packet packet; 
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);  
  addr.sin_port = 0;
  
  while (!stopThreads_) {
      // Resize the packet buffer to hold the expected packet
      packet.resize(sizeof(struct iphdr) + sizeof(protocol_header)); 
      
      socklen_t addr_len = sizeof(addr);
      ssize_t packet_size = recvfrom(sockFD_recv_, packet.data(), packet.size(), 0,
                                     reinterpret_cast<struct sockaddr*>(&addr), &addr_len);
      if (packet_size < 0) {
          perror("recvfrom");
          break;
      }
      
      // THIS WILL BE USE TO DISCARD OTHER PACKETS 
      struct iphdr* ip = reinterpret_cast<struct iphdr*>(packet.data());
      protocol_header* my_header = reinterpret_cast<protocol_header*>(packet.data() + sizeof(struct iphdr)); 
      
      // When packet arrives we store it in the recvQueue_
      // The recv funciton of the backend will fetch the data and understand 
      // what that is. 
      cout<< "IP Protocol received: " << ip->protocol << std::endl; 

      {
        // Locking mutex
        std::unique_lock<std::mutex> lock(recv_queueMutex_);
        // Add the packet to the queue 
        recvQueue_.push(packet); 
      }

      ////////////////////////////////////////
      // ADD THE NOTIFY FOR THE READ FUNCTION 
      recv_queueCV_.notify_one(); 
      ////////////////////////////////////////

      std::cout<< "Pacchetti in recvQueue_: " << recvQueue_.size() << std::endl; 

  }
}

BackendDummy::~BackendDummy() {
  // Signal threads to stop
  stopThreads_ = true;

  // Wake up any waiting threads
  send_queueCV_.notify_all();

  // Join threads
  if (sendThread_.joinable()) {
    sendThread_.join();
  }
  if (recvThread_.joinable()) {
    recvThread_.join();
  }

  // Close sockets once
  close(sockFD_recv_);
  close(sockFD_send_);

  std::cout << "BackendDummy destructor completed.\n";
}


c10::intrusive_ptr<Work> BackendDummy::send(
    std::vector<at::Tensor>& tensors,
    int dstRank,
    int tag) {
  // Create a Future for the operation.
  auto future = c10::make_intrusive<c10::ivalue::Future>(
      c10::ListType::create(c10::TensorType::get()));
      
  // Create a WorkDummy object.
  auto work = c10::make_intrusive<WorkDummy>(OpType::SEND, future);

  /* cout << tensors << std::endl; 
  cout << "tensors.size() "<<tensors.size() << std::endl; 

  for (const auto& tensor : tensors) {
        std::cout << "Tensor shape: ";
        for (const auto& size : tensor.sizes()) {
            std::cout << size << " ";
        }
        std::cout << "\nDimensions: " << tensor.dim() << std::endl;
    }
  */
  // TENSOR INPUT INFORMATION: PYTHON WILL LIKE: backend.send([tensor], 1, 100) 
  // So you have an object on which you could iterate and inside there is a list of 
  // tensor. So to get the size of the tensor you can find it inside there 
  // Still in a distributed environment you should suppose that the workers should agree
  // on the size of the tensors that they are going to send. 

  // Here you would serialize and send the tensor(s) via your socket.
  // Creating the buffer to send the packet 

 

  
  // THE tensor VARIABLE IS A LIST OF TENSORS 
  // FOR EACH TENSOR WE COMPUTE HOW MANY NUMBERS THEY HAVE 
  // TO SERIALIZE THE TENSOR YOU NEED TO USE serializeTensor
    
    // Ensure the tensor is contiguous.
    for (auto & tensor: tensors){
      tensor = tensor.contiguous();

      // Total number of elements.
      int64_t num_elements = tensor.numel();

      int n_elem_per_packet = 25; 

      int n_packets;
      if (num_elements % n_elem_per_packet == 0) {
        n_packets = num_elements / n_elem_per_packet; 
      } else {
        n_packets = num_elements / n_elem_per_packet + 1; 
      }

      cout << "Number of elements: " << num_elements << std::endl; 
      cout << "Number of packets: " << n_packets << std::endl; 
      //std::this_thread::sleep_for(std::chrono::seconds(10)); 
      // Pointer to the underlying data.
      float* data_ptr = tensor.data_ptr<float>();
      /*
      for (int i = 0; i < num_elements; ++i, ++data_ptr){
          cout << (*data_ptr) << std::endl; 
      }
      */
      float* end_ptr = data_ptr + num_elements; 

      for (int i = 0; i < n_packets; ++i){
        Packet packet; 
        //memset(&packet, 0, sizeof(packet));
        if (data_ptr + n_elem_per_packet > end_ptr ){
            BuildPacket(packet, data_ptr, end_ptr); 
        } else {
            BuildPacket(packet, data_ptr, data_ptr+n_elem_per_packet); 
        }
        // Acquire Mutex for adding packets in the queue 
        {
          std::lock_guard<std::mutex> lock(send_queueMutex_);
          sendQueue_.push(std::move(packet));
        }
        send_queueCV_.notify_one(); 
        cout<< "[SEND FUNCTION]: packet added"<< std::endl; 
        data_ptr += n_elem_per_packet ;
        
      }

    }
  


  // For this minimal example, we simply mark the future as completed.
  future->markCompleted(c10::IValue(tensors));

  return work;
}

  // HERE IMPLEMENT THE RECV FUNCTION 

  c10::intrusive_ptr<Work> BackendDummy::recv(std::vector<at::Tensor> & tensors, 
                              int dstRank, int tag){

    // CHECK THIS STUFF !!!!!!!!!!!!!!!
    // Create a Future for the operation.
    auto future = c10::make_intrusive<c10::ivalue::Future>(
    c10::ListType::create(c10::TensorType::get()));
    auto work = c10::make_intrusive<WorkDummy>(OpType::SEND, future);

    // SHOULD ADD A WAY TO ALREADY TO KNOW HOW MANY PACKETS SHOULD BE
    // RECEIVED 
    int recv_n_packets = 30; 
    /////////////////////////////////////////////////
    Packet recv_packet; 

    // FOR each tensor in the list passed by python 
    for (auto tensor: tensors){
      
      float* data = tensor.data_ptr<float>();
      int total_elements = tensor.numel(); 
      int elements_added = 0; 

      while (elements_added < total_elements){

      // SCOPE FOR THE LOCK 
      {
        std::unique_lock<std::mutex> lock(recv_queueMutex_); 
        recv_queueCV_.wait(lock, [&]{ return !recvQueue_.empty() || stopThreads_; }); 
        recv_packet = recvQueue_.front(); 
        recvQueue_.pop(); 
      }

      // RIMUOVI 
      struct iphdr * recv_ip = reinterpret_cast<iphdr*>(recv_packet.data()); 
      std::cout << "Tot_len_pacchetto: " << recv_ip->tot_len << std::endl;
      //////////////////////////////////////
      
      protocol_header* recv_header = reinterpret_cast<protocol_header*>(recv_packet.data() + sizeof(struct iphdr)); 
      for (int i = 0; i< 25 ; ++i){
        // Access all the elements and put them in the tensor 
        data[elements_added] = recv_header->data[i]; 
        ++elements_added; 
      }

      std::cout<< "Added 25 elements" << std::endl; 
    }
  }
  return work; 
  
}



} // namespace c10d


// PYBIND11_MODULE must be at global scope, outside any namespace.
PYBIND11_MODULE(dummy_collectives, m) {
  // Register BackendDummy with a lambda wrapper for the send function.
  py::class_<c10d::BackendDummy, c10::intrusive_ptr<c10d::BackendDummy>>(m, "BackendDummy")
      .def(py::init<int, int>())
      .def("send",
           [](c10d::BackendDummy &self, std::vector<at::Tensor> tensors, int dstRank, int tag) {
             // The lambda receives the vector by value (or const reference)
             // and then passes it to the actual method.
             return self.send(tensors, dstRank, tag);
           },
           "Send tensors to a destination",
           py::arg("tensors"), py::arg("dstRank"), py::arg("tag"))
      .def("recv",
            [](c10d::BackendDummy &self, std::vector<at::Tensor> tensors, int srcRank, int tag) {
              return self.recv(tensors, srcRank, tag);
            },
            "Receive tensors from a source",
            py::arg("tensors"), py::arg("srcRank"), py::arg("tag"));

  m.def("create_backend_dummy", [](int rank, int size) {
    return c10::make_intrusive<c10d::BackendDummy>(rank, size);
  });
}
