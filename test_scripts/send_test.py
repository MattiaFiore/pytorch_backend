import dummy_collectives
import torch
import time 

def main():
    # Create a BackendDummy with rank=0 and world size=1.
    backend = dummy_collectives.create_backend_dummy(0, 1)
    print("BackendDummy instance created:", backend)
    
    # Create a sample tensor.
    tensor =  torch.ones((100, 1), dtype=torch.float32)
    tensor2 =  torch.empty((25, 3), dtype=torch.float32)
    # Call the send function on the backend.
    # Here, we're sending a list containing one tensor to destination rank 1 with tag 100.
    work = backend.send([tensor], 1, 100)
    received = backend.recv([tensor2], 1, 100)
    #time.sleep(100)
    print(tensor2)
    # For this dummy backend, the send function immediately marks its operation as complete.
    # You might print the returned 'work' object or use it further depending on your design.
    print("Send function returned work object:", work)

if __name__ == '__main__':
    main()