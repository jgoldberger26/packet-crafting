#!/bin/bash

gcc -O0 -fno-stack-protector appbof/appbof.c -o appbof/appbof
gcc evil/evil.c -o evil/evil
gcc ipspoof/ipspoof.c -o ipspoof/ipspoof
gcc scapy101/scapy101.c -o scapy101/scapy101
gcc -O0 -fno-stack-protector udpbof/udpbof.c -o udpbof/udpbof