#ifndef DUMMY_H_
#define DUMMY_H_

#include <torch/csrc/distributed/c10d/Backend.hpp>
#include <torch/csrc/distributed/c10d/Work.hpp>
#include <torch/csrc/distributed/c10d/Store.hpp>
#include <c10/util/intrusive_ptr.h>
//#include <torch/csrc/jit/runtime/ivalue.h>

#include <optional>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace c10d {


// ---------------------------------------------------------------------
// WorkDummy
// ---------------------------------------------------------------------
// The base Work constructor expects:
//   Work(int rank, OpType opType, const char* backendName,
//        const std::optional<std::vector<at::Tensor>>& inputs)
// Here we pass -1 for rank, "dummy" for backendName, and std::nullopt for inputs.
class WorkDummy : public Work {
 public:
  WorkDummy(OpType opType, c10::intrusive_ptr<c10::ivalue::Future> future)
      : Work(-1, opType, "dummy", std::nullopt),
        future_(std::move(future)) {}

  bool isCompleted() override { return true; }
  bool isSuccess() const override { return true; }
  bool wait(std::chrono::milliseconds /*unused*/) override { return true; }
  c10::intrusive_ptr<c10::ivalue::Future> getFuture() override {
    return future_;
  }

 private:
  c10::intrusive_ptr<c10::ivalue::Future> future_;
};

// ---------------------------------------------------------------------
// BackendDummy
// ---------------------------------------------------------------------
class BackendDummy : public Backend {
 public:
  BackendDummy(int rank, int size, int n_register);
  ~BackendDummy(); 

  // Example override: a send operation.
  c10::intrusive_ptr<Work> send(
      std::vector<at::Tensor>& tensors,
      int dstRank,
      int tag) override;

  c10::intrusive_ptr<Work> recv(
        std::vector<at::Tensor>& tensors,
        int srcRank,
        int tag) override;

  private:
    // For a real implementation, you might store a socket descriptor here.
    int sockFD_send_; // Sending socket
    int sockFD_recv_; // Receiving socket
    std::thread sendThread_; // Sending thread 
    std::thread recvThread_; // Receiving thread 
    bool stopThreads_ = false; // Variable to stop sending and receiving threads 
    void sendingLoop();
    void receivingLoop();

    // SENDING THREAD QUEUE ATTRIBUTES
    std::queue<std::vector<char>> sendQueue_;
    std::mutex send_queueMutex_;
    std::condition_variable send_queueCV_;
    int n_register_; 
    int in_flight_packets; 

    // RECEIVING THREAD QUEUE ATTRIBUTES
    std::queue<std::vector<char>> recvQueue_; 
    std::mutex recv_queueMutex_; 
    std::condition_variable recv_queueCV_; 


    // IP address of the host 
    sockaddr_in IPV4_addr; 
    // IPv4 string 
    char ipstring[25]; 
};

} // namespace c10d

#endif