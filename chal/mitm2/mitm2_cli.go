//go:build ignore

package main

import (
	"crypto/sha256"
	"log"
	"net"
	"os"
	"time"
)

const (
	SERVER_PORT = 54321
	PING        = 1
	GET_FLAG    = 2
)

// var SHARED_KEY []byte
var CLIENT_IP = net.IPv4zero
var SERVER_IP = net.IPv4(127, 0, 0, 1)

// var SERVER_IP = net.IPv4(192, 168, 43, 21)

type appdata struct {
	counter byte
	hash    []byte
	action  byte
}

func main() {
	SHARED_SALT, err := os.ReadFile("shared_key")
	if err != nil {
		log.Fatalf("Failed to read shared key: %v", err)
	}

	counter := make([]byte, 1)

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

	hash := sha256.New()

	buffer := make([]byte, 1024)
	for {
		hash.Reset()
		hash.Write(counter)
		hash.Write(SHARED_SALT)
		data := appdata{
			counter[0],
			hash.Sum(make([]byte, 0)),
			PING,
		}

		_, err = conn.WriteToUDP(pack(data), &serverAddr)
		if err != nil {
			log.Printf("Error sending UDP packet: %v", err)
		} else {
			log.Printf("Packet sent to %s", serverAddr.IP)
		}

		conn.SetReadDeadline(time.Now().Add(5 * time.Second))
		_, remoteAddr, err := conn.ReadFromUDP(buffer)
		log.Printf("Received packet from %s", remoteAddr)
		if netErr, ok := err.(net.Error); ok && netErr.Timeout() {
			log.Println("Read timeout")
			continue
		}
		if err != nil {
			log.Printf("Error receiving UDP packet: %v", err)
			continue
		}

		counter[0] += 1
		time.Sleep(time.Second * 1)
	}
}

func pack(dat appdata) []byte {
	arr := make([]byte, 0)
	arr = append(arr, dat.action)
	arr = append(arr, dat.hash...)
	arr = append(arr, dat.counter)
	return arr
}
