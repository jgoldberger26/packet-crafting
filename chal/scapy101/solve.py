from scapy.all import send, sr1, IP, UDP, Raw, RandShort

# Hack to send to localhost
from scapy.all import conf, L3RawSocket
conf.L3socket = L3RawSocket

ACTION_PING = 0
ACTION_SET_PWD = 1
ACTION_GET_FLAG = 2


def craft_chal1(action, string):
    pkt = IP(dst="chals.labs.rpis.ec")
        
    payload = bytearray()
    payload.append(action)
    payload.append(len(string))
    payload.extend(bytes(string, 'utf-8'))
    
    pkt /= UDP(sport=RandShort(), dport = 8080)
    pkt /= Raw(payload)
    return pkt

pkt = craft_chal1(ACTION_GET_FLAG, "hello world!")
res = sr1(pkt)
print(res.show())