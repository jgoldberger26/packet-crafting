//go:build ignore

package main

import (
	"crypto/ecdh"
	"crypto/ed25519"
	"crypto/rand"
	"log"
	"net"
	"os"
	"time"
)

const (
	INIT_ACTION  byte = 1
	READY_ACTION byte = 2
	FLAG_ACTION  byte = 3
	KILL         byte = 255
	SERVER_PORT       = 54321
)

var SERVER_IP = net.IPv4(127, 0, 0, 1)
var CLIENT_IP = net.IPv4zero

// Step 1: Client Generates a key
// Step 2: Client sends key to server
// Step 3: Server generates a connection key
// Step 4: Server sends key to client, and ready signal
// Step 5: Client encrypts flag and sends it to the server

func main() {
	FLAG, err := os.ReadFile("flag.txt")
	if err != nil {
		log.Fatalf("Could not open flag: %v", err)
	}
	log.Println("Listening!")

	clientAddr := net.UDPAddr{
		IP: CLIENT_IP,
	}

	serverAddr := net.UDPAddr{
		Port: SERVER_PORT,
		IP:   SERVER_IP,
	}

	conn, err := net.ListenUDP("udp", &clientAddr)
	if err != nil {
		log.Fatalf("Failed to set up UDP listener: %v", err)
	}
	defer conn.Close()

	buffer := make([]byte, 1024)
	for {
		time.Sleep(time.Second * 10)
		// Step 1: Client generates a key
		privkey, _ := ecdh.X25519().GenerateKey(rand.Reader)
		pubkey := privkey.PublicKey()

		// Step 2: Client sends key to server
		_, err = conn.WriteToUDP(append([]byte{INIT_ACTION}, pubkey.Bytes()...), &serverAddr)
		if err != nil {
			log.Printf("Error sending UDP packet: %v", err)
		} else {
			log.Printf("Packet sent to %s", serverAddr.IP)
		}

		// Step 3: (On server side)

		// Step 4: Server sends key to client, and ready signal
		conn.SetReadDeadline(time.Now().Add(5 * time.Second))
		n, remoteAddr, err := conn.ReadFromUDP(buffer)
		if netErr, ok := err.(net.Error); ok && netErr.Timeout() {
			log.Printf("Read timeout")
			continue
		}
		log.Printf("Received packet from %s", remoteAddr)
		if err != nil {
			log.Printf("Error receiving UDP packet: %v", err)
			continue
		}
		if n != 1+ed25519.PublicKeySize {
			log.Printf("Invalid packet recieved")
			continue
		}

		// Step 5: Client encrypts flag and sends it to the server
		if buffer[0] != READY_ACTION {
			continue
		}
		serverPubkey, _ := ecdh.X25519().NewPublicKey(buffer[1 : ed25519.PublicKeySize+1])
		sharedSecret, err := privkey.ECDH(serverPubkey)
		if err != nil {
			log.Printf("Invalid public key: %v", err)
			continue
		}

		encryptedFlag := FLAG
		for i := range encryptedFlag {
			encryptedFlag[i] ^= sharedSecret[i]
		}

		_, err = conn.WriteToUDP(append([]byte{FLAG_ACTION}, encryptedFlag...), remoteAddr)
		if err != nil {
			log.Printf("Failed to send data: %v", err)
		}
	}
}
