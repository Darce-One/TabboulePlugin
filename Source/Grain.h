/*
  ==============================================================================

    Grain.h
    Created: 16 Apr 2022 5:42:07pm
    Author:  B162025

  ==============================================================================
*/

#pragma once
#include "Oscillator.h"


/**=============================================================================
 Grain is an object representing a grain in a granulation synthesis.
 
 The Grain class is meant to be used in conjunction with the GrainBuffer class, where the buffer is stored.
 
 The Grain object is driven by a phasor whose period represents the grain size.
 
 When a grain is done playing (Phasor resets), it looks gets the new MaxReadPos from the grainBuffer
 instance, and moves the readPos to a new location.
 
 Remember to intialise the class in prepareToPlay() by setting the sample rate.
 */
class Grain
{
public:
     
    ///Constructor, sets the sampleRate
    Grain (int _sampleRate)
    {
        setSampleRate (_sampleRate);
    }
    
    ///Constructor, sets the sampleRate, grainPhase and grainPeriod.
    Grain (int _sampleRate, float initialPhase, float grainPeriod)
    {
        setSampleRate (_sampleRate);
        setGrainPhase (initialPhase);
        setGrainPeriod (grainPeriod);
    }
    
    /**
     Processes the grain instance, incrementing the readPos as necessary.
     
     This method must be called at every sample, for each grain. Only then can the other methods
     be called to avoid errors.
     
     @param _grainPeriod                            length of grain in seconds.
     
     @param _grainMaxReadPos                    The last readable sample from the GrainBuffer instance.
     
     @param _grainRandomisation             Scalar float in range [0, 1], denoting how randomly the grains are selected.
     
     @param _shape                                          Scalar float in range [0, 1], denoting how steep the fade ins and outs the grains obey.
     
     @param _chanceToSkip                           Probability of a skipped grain.
     
     @param _stereoRandomness                  Scalar float in range [0-1] denoting the width of the stereo field.
     */
    void process (float _grainPeriod, int _grainMaxReadPos, float _grainRandomisation, float _shape, float _chanceToSkip, float _stereoRandomness)
    {
        
        readPos++;
        setGrainPeriod (_grainPeriod);
        sampleEnvelope = std::min ((20.0f * _shape + 1.0f) * triRamp.process(), 1.0f);
        timeToReset = triRamp.newCycleStarted();
        
        if (timeToReset == true)
        {
            maxReadPos = _grainMaxReadPos;
            readPos = readPos + floor ((random.nextFloat()-0.5f) * maxReadPos * _grainRandomisation);
            
            if (random.nextFloat() < _chanceToSkip)
            {
                skippedGrainVolume = 0.0f;
            }
            else
            {
                skippedGrainVolume = 1.0f;
            }
            
            stereoVolumeLeft = 0.5f + ((random.nextFloat() - 0.5f) * _stereoRandomness);
            stereoVolumeRight = 1.0f - stereoVolumeLeft;
            
        }
        
        if (readPos >= maxReadPos || readPos < 0)
            readPos = 0;
        
    }
    
    /// Returns the sample of the buffer to read from.
    float getReadPos()
    {
        return readPos;
    }
    
    /// Returns the envelope coefficient at this sample for the grain.
    float getSampleEnvelope()
    {
        return sampleEnvelope * skippedGrainVolume;
    }
    
    /// Returns the envelope coefficient at this sample for the grain, allowing for FFT functionality.
    float getSineSquaredEnvelope()
    {
        float sinEnvelope = sin (triRamp.getPhase() * 3.14159265359);
        return sinEnvelope * sinEnvelope;
    }
    
    /// Returns the volume of the left sample
    float getStereoVolumeLeft()
    {
        return sampleEnvelope * skippedGrainVolume * stereoVolumeLeft;
    }
    
    /// Returns the volume of the right sample;
    float getStereoVolumeRight()
    {
        return sampleEnvelope * skippedGrainVolume * stereoVolumeRight;
    }
    
    /**
     Sets the length of the grain in seconds.
     
     Thanks to the architecture of the grains being built on top of a triangular wave, the length of the grain can be changed without harm mid-grain.
     
     @param grainPeriod length of grain in seconds.
     */
    void setGrainPeriod(float grainPeriod)
    {
        triRamp.setFrequency(1.0f/grainPeriod);
    }
    
    void setGrainPhase(float phase)
    {
        /// Sets the phase of the grain. Method to be used lightly, as clicks may occur from envelopes and unexpectedly long grains.
        triRamp.setPhase(phase);
    }
    
    void setSampleRate(int _sampleRate)
    {
        /// Sets the sample rate of the project.
        triRamp.setSampleRate(_sampleRate);
        sampleRate = _sampleRate;
    }
    
    bool newGrainStarted()
    {
        return triRamp.newCycleStarted();
    }
    
    //==========================================================================
private:
    int sampleRate;
    juce::Random random;
    TriRamp triRamp;
    int readPos = 0;
    bool timeToReset = false;
    int maxReadPos = 4410;      //initialisation to be overriden before playback in process method.
    float sampleEnvelope = 0.0f;
    float skippedGrainVolume = 1.0f;
    float stereoVolumeLeft = 1.0f;
    float stereoVolumeRight = 1.0f;
};

/**=============================================================================
 Small class with just a handful of methods to make the process of calculating individual volume and
 phase data for each of the 5 grains.
 
 It is not necessary to call managePhases every sample, once a buffer should be fine.
 If extremely precise Grain control is needed, can call all three methods once per sample.
 */
class GrainManager
{
public:
    
    /**
     Method to be called once at a time, calculates the volume and spacing of the grains
     
     @param _activeGrains float in range:  [1,  4.99], whose floored value is the number of grains.
     */
    void managePhases(float _activeGrains)  // active grains range from 1 to 4.99
    {
        for (int i=0; i<5; i++)
        {
            phases[i] = i * (1.0f / floor(_activeGrains + 1));
            volumes[i] = std::min(1.0f, std::max(_activeGrains - i, 0.0f));
        }
    }
    
    float getPhaseForGrain(int index)
    {
        /// Returns the respective phase of the indexed grain.
        return phases[index];
    }
    
    float getVolumeForGrain(int index)
    {
        /// Returns the respective volumes of the indexed grain.
        return volumes[index];
    }
    
    //==========================================================================
private:
    float volumes[5] = {0, 0, 0, 0, 0};
    float phases[5] = {0, 0, 0, 0, 0};
};
