//go:build ignore

package main

import (
	"log"
	"net"
)

const SERVER_PORT = 54321
const CLIENT_PORT = 54320
const FLAG2 = "b3tw3en}"

// var CLIENT_IP = net.IPv4(192, 168, 43, 21)
var CLIENT_IP net.IP = net.IPv4(127, 0, 0, 1)
var SERVER_IP net.IP = nil

func main() {
	log.Println("Listening!")

	serverAddr := net.UDPAddr{
		Port: SERVER_PORT,
		IP:   SERVER_IP,
	}

	clientAddr := net.UDPAddr{
		Port: CLIENT_PORT,
		IP:   CLIENT_IP,
	}

	conn, err := net.ListenUDP("udp", &serverAddr)
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

		_, err = conn.WriteToUDP([]byte(FLAG2), &clientAddr)
		if err != nil {
			log.Printf("Error sending UDP packet: %v", err)
		} else {
			log.Printf("Packet sent to %s", clientAddr.IP)
		}
	}
}
