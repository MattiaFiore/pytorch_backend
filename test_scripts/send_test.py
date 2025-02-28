import dummy_collectives
import torch

def main():
    # Create a BackendDummy with rank=0 and world size=1.
    backend = dummy_collectives.create_backend_dummy(0, 1)
    print("BackendDummy instance created:", backend)
    
    # Create a sample tensor.
    tensor = torch.tensor([
    [1, 2, 3, 1, 2, 3],
    [4, 5, 6, 1, 2, 3],
    [7, 8, 9, 1, 2, 3], 
    [7, 8, 9, 1, 2, 3], 
    [7, 8, 9, 1, 2, 3]
    ], dtype=torch.float32)
    
    # Call the send function on the backend.
    # Here, we're sending a list containing one tensor to destination rank 1 with tag 100.
    work = backend.send([tensor, tensor], 1, 100)
    
    # For this dummy backend, the send function immediately marks its operation as complete.
    # You might print the returned 'work' object or use it further depending on your design.
    print("Send function returned work object:", work)

if __name__ == '__main__':
    main()
