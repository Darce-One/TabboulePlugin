/*
  ==============================================================================

    Grain.h
    Created: 16 Apr 2022 5:42:07pm
    Author:  Andreas Papaeracleous

  ==============================================================================
*/

#pragma once
#include "Oscillator.h"


/**
 Grain is an object representing a grain in a granulation.
 The Grain class is meant to be used in conjunction with the GrainBuffer class, where the buffer is stored
 The Grain object is driven by a phasor whose period represents the grain size.
 When a grain is done playing (Phasor resets), it looks gets the new MaxReadPos from the grainBuffer instance, and moves the readPos to a new location.
 */
class Grain
{
public:
    
    float getReadPos()
    {
        return readPos;
    }
    
    void process(int _grainMaxReadPos, float _grainRandomisation, float _shape)
    {
        sampleEnvelope = std::max(_shape * triOscillator.process(), 1.0f);
        readPos++;
        timeToReset = triOscillator.newCycleStarted();
        
        if (readPos >= maxReadPos)
            readPos = 0;
        
        if (timeToReset == true)
        {
            maxReadPos = _grainMaxReadPos;
            readPos = readPos + floor((random.nextFloat()-0.5f) * maxReadPos * _grainRandomisation);
        }
    }
    
    float getSampleEnvelope()
    {
        return sampleEnvelope;
    }
    
    void setGrainPeriod(float grainPeriod)
    {
        triOscillator.setFrequency(1.0f/grainPeriod);
    }
    
    void setGrainPhase(float phase)
    {
        triOscillator.setPhase(phase);
    }
    
    void setSampleRate(int _sampleRate)
    {
        triOscillator.setSampleRate(_sampleRate);
        sampleRate = _sampleRate;
    }
    
    
private:
    int sampleRate;
    juce::Random random;
    TriOsc triOscillator;
    int readPos = 0;
    bool timeToReset = false;
    int maxReadPos = 4410;
    float sampleEnvelope;
};
