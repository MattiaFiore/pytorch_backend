import dummy_collectives
import torch

def main():
    # Create a BackendDummy with rank=0 and world size=1.
    backend = dummy_collectives.create_backend_dummy(0, 1)
    print("BackendDummy instance created:", backend)
    
    # Since our dummy backend doesn't expose a send method to Python,
    # you can only verify that the backend was constructed.
    # In a complete implementation, you would register the backend with torch.distributed
    # and test collective operations.

if __name__ == '__main__':
    main()