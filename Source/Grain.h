/*
  ==============================================================================

    Grain.h
    Created: 16 Apr 2022 5:42:07pm
    Author:  Andreas Papaeracleous

  ==============================================================================
*/

#pragma once
#include "Oscillator.h"


class Grain
{
public:
    
    Grain(int _sampleRate)
    {
        sampleRate = _sampleRate;
        
    }
    
    
    
    
    
    
private:
    int sampleRate;
    juce::Random random;
    Phasor phasor;
};
