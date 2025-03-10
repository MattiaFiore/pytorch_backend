from p4utils.utils.helper import load_topo
from p4utils.utils.sswitch_thrift_API import *
from p4utils.utils.thrift_API import ThriftAPI
import time 
import grpc
from p4.v1 import p4runtime_pb2
from p4.v1 import p4runtime_pb2_grpc
from p4.config.v1 import p4info_pb2
import argparse

topo = load_topo('topology.json')
controllers = {}

#port is 9090
thrift_port = 9090

for switch, data in topo.get_p4switches().items():
    print(data)
    thrift_ip = topo.get_thrift_ip(switch)
    controllers[switch] = SimpleSwitchThriftAPI(thrift_port=data['thrift_port'],
                                                   json_path=data['json_path'])

client = ThriftAPI(thrift_port, thrift_ip, pre_type = None)
counter = 0

register_size = 100

parser = argparse.ArgumentParser()
parser.add_argument('-t', '--time_update', type = int, help='[CONTROLLER]: Time of update', required=False, default = 5)
args = parser.parse_args()

while True: 

    print(f'Iterazione: {counter}', end = " ")
    for i in range(register_size):
        if i % register_size == 0: 
            print()
        value = client.register_read('pool', index=i)
        print(value, end = " ")
    print()
    print("COUNTER: ")
    for i in range(register_size):
        if i % register_size == 0: 
            print()
        value = client.register_read('counter_pool', index=i)
        print(value, end = " ")
    print()
    counter+=1
    print('Waiting ...')
    time.sleep(args.time_update)