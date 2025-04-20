//go:build ignore

package main

import (
	"log"
	"net"
	"time"
)

const FLAG1 = "RPISEC{m4n_in"
const SERVER_PORT = 54321
const CLIENT_PORT = 54320

// var SERVER_IP = net.IPv4(192, 168, 43, 21)
var SERVER_IP net.IP = net.IPv4(127, 0, 0, 1)
var CLIENT_IP net.IP = net.IPv4(127, 0, 0, 1)

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

	conn, err := net.ListenUDP("udp", &clientAddr)
	if err != nil {
		log.Fatalf("Failed to set up UDP listener: %v", err)
	}
	defer conn.Close()

	buffer := make([]byte, 1024)
	for {
		time.Sleep(time.Second * 5)
		_, err = conn.WriteToUDP([]byte(FLAG1), &serverAddr)
		if err != nil {
			log.Printf("Error sending UDP packet: %v", err)
		} else {
			log.Printf("Packet sent to %s", serverAddr.IP)
		}

		conn.SetReadDeadline(time.Now().Add(5 * time.Second))
		_, remoteAddr, err := conn.ReadFromUDP(buffer)
		if netErr, ok := err.(net.Error); ok && netErr.Timeout() {
			log.Println("Read timeout")
			continue
		} // Blocking
		log.Printf("Received packet from %s", remoteAddr)
		if err != nil {
			log.Printf("Error receiving UDP packet: %v", err)
			continue
		}
	}
}
