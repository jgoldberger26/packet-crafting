package main

import (
	"log"
	"net"
)

const TRIGGER_PORT = 54320
const CLIENT_PORT = 54321
const FLAG = "RPISEC{l4y3r_2_pwn}"

var CLIENT_IP = net.IPv4(10, 42, 0, 200)

func main() {
	log.Println("Listening!")
	triggerAddr := net.UDPAddr{
		Port: TRIGGER_PORT,
		IP:   net.IPv4zero,
	}

	conn, err := net.ListenUDP("udp", &triggerAddr)
	if err != nil {
		log.Fatalf("Failed to set up UDP listener: %v", err)
	}
	defer conn.Close()

	buffer := make([]byte, 1024)
	for {
		_, remoteAddr, err := conn.ReadFromUDP(buffer) // Blocking
		if err != nil {
			log.Printf("Error receiving UDP packet: %v", err)
			continue
		}

		log.Printf("Received packet from %s", remoteAddr)

		clientAddr := net.UDPAddr{
			Port: CLIENT_PORT,
			IP:   CLIENT_IP,
		}

		_, err = conn.WriteToUDP([]byte("Packet received"), &clientAddr)
		if err != nil {
			log.Printf("Error sending UDP packet: %v", err)
		} else {
			log.Printf("Packet sent to %s", clientAddr.IP)
		}
	}
}
