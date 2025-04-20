//go:build ignore

package main

import (
	"crypto/ecdh"
	"crypto/ed25519"
	"crypto/rand"
	"log"
	"net"
)

const (
	INIT_ACTION  byte = 1
	READY_ACTION byte = 2
	FLAG_ACTION  byte = 3
	KILL         byte = 255
	SERVER_PORT       = 54321
)

const FLAG = "RPISEC{m0nk3y_1n_th3_m1ddl3}"

var PUBLIC_KEY, PRIVATE_KEY, _ = ed25519.GenerateKey(nil)

// Step 1: Client Generates a key
// Step 2: Client sends key to server
// Step 3: Server generates a connection key
// Step 4: Server sends key to client, and ready signal
// Step 5: Client encrypts flag and sends it to the server

func main() {
	log.Println("Listening!")
	triggerAddr := net.UDPAddr{
		Port: SERVER_PORT,
		IP:   net.IPv4zero,
	}

	conn, err := net.ListenUDP("udp", &triggerAddr)
	if err != nil {
		log.Fatalf("Failed to set up UDP listener: %v", err)
	}
	defer conn.Close()

	buffer := make([]byte, 1024)
	for {
		// Step 1: Client generates a key (On client side)
		// Step 2: Client sends key to server (On client side)
		// Step 3: Server generates a connection key

		// Step 4: Server sends a key to the client, and a ready signal
		n, remoteAddr, err := conn.ReadFromUDP(buffer)
		if err != nil {
			log.Println(buffer)
			log.Printf("Error receiving UDP packet: %v", err)
			continue
		}

		log.Printf("Received packet from %s", remoteAddr)

		if n > 0 && buffer[0] == FLAG_ACTION {
			log.Println("Flag recieved, no further action needed.")
			continue
		} else if n != 1+ed25519.PublicKeySize || buffer[0] != INIT_ACTION {
			log.Printf("Invalid packet recieved")
			continue
		}

		privkey, _ := ecdh.X25519().GenerateKey(rand.Reader)
		pubkey := privkey.PublicKey()

		_, err = conn.WriteToUDP(append([]byte{READY_ACTION}, pubkey.Bytes()...), remoteAddr)
		if err != nil {
			log.Printf("Error sending UDP packet: %v", err)
		} else {
			log.Printf("Packet sent to %s", remoteAddr.IP)
		}
	}
}
