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
#define RECEIVER_PORT "4967" // Transmitters must connect to this port!

#pragma mark TRANSMITTER

class UdpAudioTransmitter {
public:
    UdpAudioTransmitter() { create_socket(); }
    
    void init(){
        if(!socketIsOpen) { create_socket(); }
        for(int ch = 0; ch < MAX_CHANNELS; ch++) {
            sampleIx[ch] = 0;
            msgBufIx[ch] = 0;
        }
    }
    
    


    
    // Called for each sample of every channel:
    void copySampleToMsg(const float *audioBuffer, int frameOffset, int channel){
        if(sampleIx[channel] == 0) {
            initNewAudioMsg(channel);
        }
        msgBufPtr = (float *)&msgBuf[channel][msgBufIx[channel]];
        *msgBufPtr = audioBuffer[frameOffset];
        msgBufIx[channel] += sizeof(float);
        sampleIx[channel] += 1;
        
        if(sampleIx[channel] == SAMPLES_PR_MSG) {
            transmit(channel);
            sampleIx[channel] = 0;
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
    
    char msgBuf[MAX_CHANNELS][(sizeof(float) * SAMPLES_PR_MSG) + (sizeof(int) * 10)];
    int msgBufIx[MAX_CHANNELS];
    float *msgBufPtr;
    int sampleIx[MAX_CHANNELS];
    uint16_t seqNr;

    
    
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
            
            // Here bind happens if receiver
            break;
        }

        if(p == NULL){
          //fprintf(stderr, "Failed to create socket\n");
          return 2;
        }
        
        //printf("Transmitter: Socket was successfully set up!\n");
        socketIsOpen = true;
    
        seqNr = 0;
        
        
        return 0;
    }
    
    // Called before copying frames into the message buffer:
    void initNewAudioMsg(int channel) {
            sampleIx[channel] = 0;
            msgBufIx[channel] = 0;
        
        // Copying into the message buffer:
        memcpy(&msgBuf[channel][0], &seqNr, sizeof(uint16_t));
        msgBufIx[channel] += sizeof(uint16_t);
        
        uint16_t fc = (uint16_t)SAMPLES_PR_MSG;
        memcpy(&msgBuf[channel][msgBufIx[channel]], &fc, sizeof(uint16_t));
        msgBufIx[channel] += sizeof(uint16_t);
        
        uint8_t ch = (uint16_t)channel;
        memcpy(&msgBuf[channel][msgBufIx[channel]], &ch, sizeof(uint16_t));
        msgBufIx[channel] += sizeof(uint16_t);
        
        if(channel == 0) { seqNr = ++seqNr % UINT16_MAX; }
    }
    
    // Called after a message has been filled with samples:
    void transmit(int channel){
        if(socketIsOpen){
            numbytes = sendto(sockfd, &msgBuf[channel][0], msgBufIx[channel], 0, p->ai_addr, p->ai_addrlen);
        }
    }
};



