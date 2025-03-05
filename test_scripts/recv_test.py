#!/usr/bin/env python3
import struct
from scapy.all import *
from scapy.packet import Packet, Field

class MyFloatField(Field):
    def __init__(self, name, default):
        super().__init__(name, default, fmt="!f")  # network-order float
    def addfield(self, pkt, s, val):
        return s + struct.pack("!f", val)
    def getfield(self, pkt, s):
        return s[4:], struct.unpack("!f", s[:4])[0]

class MyCustomHeader(Packet):
    name = "MyCustomHeader"
    fields_desc = [MyFloatField("field%d" % i, float(i)) for i in range(1, 26)]

packet = IP(dst="10.0.0.1", proto=253) / MyCustomHeader()
packet.show()
send(packet)
