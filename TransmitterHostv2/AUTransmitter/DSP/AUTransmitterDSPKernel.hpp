//
//  AUTransmitterDSPKernel.hpp
//  AUTransmitter
//
//  Created by Hallgrim Bratberg on 11/08/2021.
//

#ifndef AUTransmitterDSPKernel_hpp
#define AUTransmitterDSPKernel_hpp

#import "DSPKernel.hpp"

#import "GrimsUdpLib.hpp"

enum {
    paramOne = 0,
};

/*
 AUTransmitterDSPKernel
 Performs simple copying of the input signal to the output.
 As a non-ObjC class, this is safe to use from render thread.
 */
class AUTransmitterDSPKernel : public DSPKernel {
public:
    
    // MARK: Member Functions

    AUTransmitterDSPKernel() {}

    void init(int channelCount, double inSampleRate) {
        chanCount = channelCount;
        sampleRate = float(inSampleRate);
        
        trm.init();
    }

    void reset() {
     // Called when audio stream stops
    }
    
    void deallocate() {
        trm.deallocate();
    }

    bool isBypassed() {
        return bypassed;
    }

    void setBypass(bool shouldBypass) {
        bypassed = shouldBypass;
    }

    void setParameter(AUParameterAddress address, AUValue value) {
        switch (address) {
            case paramOne:

                break;
        }
    }

    AUValue getParameter(AUParameterAddress address) {
        switch (address) {
            case paramOne:
                // Return the goal. It is not thread safe to return the ramping value.
                return 0.f;

            default: return 0.f;
        }
    }

    void setBuffers(AudioBufferList* inBufferList, AudioBufferList* outBufferList) {
        inBufferListPtr = inBufferList;
        outBufferListPtr = outBufferList;
    }

    void process(AUAudioFrameCount frameCount, AUAudioFrameCount bufferOffset) override {
        if (bypassed) {
            // Pass the samples through
            for (int channel = 0; channel < chanCount; ++channel) {
                if (inBufferListPtr->mBuffers[channel].mData ==  outBufferListPtr->mBuffers[channel].mData) {
                    continue;
                }
                
                for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                    const int frameOffset = int(frameIndex + bufferOffset);
                    const float* in  = (float*)inBufferListPtr->mBuffers[channel].mData  + frameOffset;
                    float* out = (float*)outBufferListPtr->mBuffers[channel].mData + frameOffset;
                    *out = *in;
                }
            }
            return;
        }
        
        if(!framesRcvdSet){
            framesRcvd = frameCount;
            printf("Transmitter, Framecount: %d\n", frameCount);
            framesRcvdSet = true;
        }
        
        // Perform per sample dsp on the incoming float *in before assigning it to *out
        for (int channel = 0; channel < chanCount; ++channel) {
            
            // Get pointer to immutable input buffer and mutable output buffer
            const float* in = (float*)inBufferListPtr->mBuffers[channel].mData;
            //float* out = (float*)outBufferListPtr->mBuffers[channel].mData;
            
            for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                const int frameOffset = int(frameIndex + bufferOffset);
                
                // Do your sample by sample dsp here...
                //out[frameOffset] = in[frameOffset];
                
                // Copying the single channel sample into the message:
                trm.copySampleToMsg(in, frameOffset, channel);
            }
        }
    }

    // MARK: Member Variables

private:
    int chanCount = 0;
    float sampleRate = 44100.0;
    bool bypassed = false;
    AudioBufferList* inBufferListPtr = nullptr;
    AudioBufferList* outBufferListPtr = nullptr;
    
    UdpAudioTransmitter trm{};
    bool framesRcvdSet = false;
    int framesRcvd = 0;
    
};

#endif /* AUTransmitterDSPKernel_hpp */
