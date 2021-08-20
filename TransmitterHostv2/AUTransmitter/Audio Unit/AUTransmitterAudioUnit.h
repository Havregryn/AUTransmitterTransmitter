//
//  AUTransmitterAudioUnit.h
//  AUTransmitter
//
//  Created by Hallgrim Bratberg on 11/08/2021.
//

#import <AudioToolbox/AudioToolbox.h>
#import "AUTransmitterDSPKernelAdapter.h"

// Define parameter addresses.
extern const AudioUnitParameterID myParam1;

@interface AUTransmitterAudioUnit : AUAudioUnit

@property (nonatomic, readonly) AUTransmitterDSPKernelAdapter *kernelAdapter;
- (void)setupAudioBuses;
- (void)setupParameterTree;
- (void)setupParameterCallbacks;
@end
