//
//  GrimsUdpLib.hpp
//  UDPAU
//
//  Created by Hallgrim Bratberg on 08/08/2021.
//

#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// For testing only:
#include <math.h>

#include <fcntl.h>



#define SAMPLES_PR_MSG 32
#define MAX_CHANNELS 2
#define RECEIVER_NAME "grims-mac.local"
#define RECEIVER_PORT "4973" // Transmitters must connect to this port!

#pragma mark TRANSMITTER

class UdpAudioTransmitter {
public:
    UdpAudioTransmitter() { create_socket(); }
    
    void init(int channels){
        this->channels = (uint16_t) channels;
        if(!socketIsOpen) { create_socket(); }
        for(int ch = 0; ch < MAX_CHANNELS; ch++) {
            sampleIx[ch] = 0;
            msgBufAudioStart[ch] = &msgBuf[sizeof(uint16_t) * 3 + ch * sizeof(float) * SAMPLES_PR_MSG];
        }
        msgSize = sizeof(uint16_t) * 3 + channels * sizeof(float) * SAMPLES_PR_MSG;
        
        seqNr = 0;
    }
    
    // Called for each sample of every channel:
    void copySampleToMsg(const float *audioBuffer, int frameOffset, int channel){
        if(channel == 0 && sampleIx[0] == 0) {
            initNewAudioMsg();
        }
        
        float* msgBufPtr = (float *)&msgBufAudioStart[channel][sampleIx[channel] * sizeof(float)];
        *msgBufPtr = audioBuffer[frameOffset];
        sampleIx[channel] += 1;
        
        if(channel == channels - 1 && sampleIx[channel] == SAMPLES_PR_MSG) {
            transmit();
            for(int ch = 0; ch < channels; ch++) {
                sampleIx[ch] = 0;
            }
        }
        
    }
    
    void deallocate(){
        if(socketIsOpen){
            close(sockfd);
            socketIsOpen = false;
            //printf("Transmitter: Socket was closed!\n");
        }
    }
    
    
private:
    // Socket and udp variables:
    int sockfd;
    bool socketIsOpen = false;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    long numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    
    char msgBuf[sizeof(float) * SAMPLES_PR_MSG * MAX_CHANNELS + sizeof(int) * 10];
    char* msgBufAudioStart[MAX_CHANNELS];
    int sampleIx[MAX_CHANNELS];
    uint16_t seqNr;
    uint16_t channels;
    int msgSize;

    
    
    // Private functions:
    int create_socket(void){
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_DGRAM;
        
        if((rv = getaddrinfo(RECEIVER_NAME, RECEIVER_PORT, &hints, &servinfo)) != 0){
          fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
          return 1;
        }

        /* Looping through all results, use the first working socket */
        for(p = servinfo; p != NULL; p = p->ai_next){
            if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
                perror("talker: socket");
                continue;
            }
            break;
        }

        if(p == NULL){
          //fprintf(stderr, "Failed to create socket\n");
          return 2;
        }
        
        //printf("Transmitter: Socket was successfully set up!\n");
        socketIsOpen = true;
    
        
        
        return 0;
    }
    
    // Called before copying frames into the message buffer:
    void initNewAudioMsg() {
        
        // Copying into the message buffer:
        memcpy(&msgBuf[0], &seqNr, sizeof(uint16_t));
        
        uint16_t fc = (uint16_t)SAMPLES_PR_MSG;
        memcpy(&msgBuf[sizeof(uint16_t)], &fc, sizeof(uint16_t));
        
        uint16_t ch = (uint16_t)channels;
        memcpy(&msgBuf[sizeof(uint16_t) * 2], &ch, sizeof(uint16_t));
        //if(seqNr < 10) printf("Sent msg seqNr %d with %d channels\n", seqNr, ch);
        
        
        seqNr = ++seqNr % UINT16_MAX;
    }
    
    // Called after a message has been filled with samples:
    void transmit(){
        if(socketIsOpen){
            numbytes = sendto(sockfd, &msgBuf[0], msgSize, 0, p->ai_addr, p->ai_addrlen);
        }
    }
};


// New version: All channels into one message

//

//

/*
 New version: All channels into one message
 seqNr, frameCount, channels, audio channel 0, audio channel 1...
 EN peker som peker på starten av hvert kanalområde i buffer.
 
 Skrive til buffer:
 
 Init ny melding dersom ikke finnes fra før:
    Skriv inn seqNr, frameCOunt, channels i buffer.
 
 skrive audio i buffer:
    Kopier til msgBufAudioStart + bufIx[channel] * sizeof(float)
 
 Dersom bufIx har nådd meldingsgrensen: SEND, sett bufIx = 0;
    
 **/


