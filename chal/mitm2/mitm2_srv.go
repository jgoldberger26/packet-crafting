//go:build ignore

package main

import (
	"crypto/sha256"
	"errors"
	"log"
	"net"
	"os"
	"slices"
)

const (
	LISTEN_PORT = 54321
	PING        = 1
	GET_FLAG    = 2
)

type appdata struct {
	counter byte
	hash    []byte
	action  byte
}

func main() {
	log.Println("Listening!")
	SHARED_SALT, err := os.ReadFile("shared_key")
	if err != nil {
		log.Fatalf("Could not open shared key: %v", err)
	}
	FLAG, err := os.ReadFile("flag.txt")
	if err != nil {
		log.Fatalf("Could not open flag: %v", err)
	}

	triggerAddr := net.UDPAddr{
		Port: LISTEN_PORT,
		IP:   net.IPv4zero,
	}

	conn, err := net.ListenUDP("udp", &triggerAddr)
	if err != nil {
		log.Fatalf("Failed to set up UDP listener: %v", err)
	}
	defer conn.Close()

	hash := sha256.New()
	buffer := make([]byte, 1024)

	for {
		var returnString string

		n, remoteAddr, err := conn.ReadFromUDP(buffer)
		if err != nil {
			log.Printf("Error receiving UDP packet: %v", err)
			continue
		}

		log.Printf("Received packet from %s", remoteAddr)
		appdat, err := unpack(buffer[0:n])
		if err != nil {
			log.Printf("Error unpacking packet: %v", err)
			returnString = "Malformed packet."
			goto sendmsg
		}

		hash.Reset()
		hash.Write([]byte{appdat.counter})
		hash.Write(SHARED_SALT)

		{
			calculated := hash.Sum(make([]byte, 0))

			if slices.Compare(calculated, appdat.hash) != 0 {
				returnString = "Bad hash!"
			} else if appdat.action == PING {
				returnString = "Authenticated! Pong!"
			} else if appdat.action == GET_FLAG {
				returnString = "Authenticated! The flag is " + string(FLAG)
			} else {
				returnString = "Authenticated! invalid action."
			}
		}

	sendmsg:
		_, err = conn.WriteToUDP([]byte(returnString), remoteAddr)
		if err != nil {
			log.Printf("Error sending UDP packet: %v", err)
		} else {
			log.Printf("Packet sent to %s", remoteAddr.IP)
		}
	}
}

func unpack(dat []byte) (appdata appdata, err error) {
	if len(dat) != 1+sha256.Size+1 {
		err = errors.New("Invalid pack length")
		return
	}
	appdata.counter = dat[0]
	appdata.hash = dat[1 : sha256.Size+1]
	appdata.action = dat[sha256.Size+1]
	return
}
