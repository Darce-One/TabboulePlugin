/*
  ==============================================================================

    Oscillator.h
    Created: 16 Apr 2022 5:42:07pm
    Author:  B162025

  ==============================================================================
*/

#pragma once
#include <cmath>
#include "CustomFunctions.h"

/**
 Creates a Phasor instance.
 It takes in a frequency and a sample rate and returns a phase between 0 and 1.
 The process() method must be run once (and only once) at every sample.
 
 Used as a base class for many oscillator types, including Sine, Square, Triangle and Saw.
 */
class Phasor
{
public:
    
    virtual ~Phasor() {}
    
    ///sets the sample rate of the oscillator
    void setSampleRate (float _sampleRate)
    {
        sampleRate = _sampleRate;
    }
    
    ///sets the frequency of the oscillator
    void setFrequency (float _frequency)
    {
        frequency = _frequency;
        phaseDelta = frequency / sampleRate;
    }
    
    ///resets the phase of the phasor
    void setPhase (float p)
    {
        phase = p;
    }
    
    /// Returns the phase of the oscillator. Useful for getting more options out of one Oscillator instance.
    float getPhase()
    {
        return phase;
    }
    
    ///Process the next sample of the sinewave oscillator:
    float process()
    {
        // Update Phase
        phase += phaseDelta;
        
        // Keep phase in range 0-1
        if (phase > 1.0f)
        {
            phase -= 1.0f;
            newCycle = true;
        }
        
        else
        {
            newCycle = false;
        }
        
        return output (phase);
    }
    
    ///Determines what happens to the phase
    ///In the phasor class, it doesn't alter it
    virtual float output (float _phase)
    {
        
        return _phase;
    }
    
    ///returns true at every new phase cycle;
    bool newCycleStarted()
    {
        return newCycle;
    }
    
    float getPhaseDelta()
    {
        return phaseDelta;
    }
    
    //==========================================================================
private:
    float sampleRate = 44100.0f;
    float phase = 0.0f;
    float phaseDelta;
    float frequency = 1.0f;
    bool newCycle = true;
};

//==============================================================================
/**
 Creates a triangle ramp instance: goes from 0, to 1 and back to 0.
 The process() method must be run once (and only once) at every sample.
 */
class TriRamp : public Phasor
{
    float output (float _phase) override
    {
        if (_phase <= 0.5f)
        {
            return 2.0f * _phase;
        }
        
        else
        {
            return 2.0f * (1.0f - _phase);
        }
    }
};


//==============================================================================
/**
 Creates a triangle oscillator instance.
 The process() method must be run once (and only once) at every sample.
 */
class TriOsc : public Phasor
{
    float output (float _phase) override
    {
        return  4.0 * (fabs (_phase - 0.5f) - 0.25f);
    }
};

//==============================================================================
/**
 Creates a sine oscillator instance.
 The process() method must be run once (and only once) at every sample.
 */
class SineOsc : public Phasor
{
    float output (float _phase) override
    {
        return sin (_phase * 2 * 3.14159265359);
    }
};

//==============================================================================
/**
 Creates a perfect square oscillator instance with hard edges.
 The process() method must be run once (and only once) at every sample.
 */
class HardSquareOsc : public Phasor
{
    ///Creates a square oscillator instance
    public : void setPulseWidth (float _width)
    {
        width = _width;
    }
    
    float output (float _phase) override
    {
        if (_phase < width)
            return -1.0f;
        
        else
        {
            return 1.0f;
        }
    }
    
private:
    float width = 0.5;
};

//==============================================================================
/**
 Creates a smoother square oscillator instance with soft edges.
 The process() method must be run once (and only once) at every sample.
 */
class SoftSquareOsc : public Phasor
{
    public : void setPulseWidth (float _width)
    {
        width = _width;
    }
    
    float output (float _phase) override
    {
        return -1 * tanhf (50 * (2 * fabs(0.5 - width) + sin (2 * 3.14159265359 * _phase)));
    }
    
private:
    float width = 0.5;
};

//==============================================================================
/**
 Creates a sawtooth oscillator instance.
 The process() method must be run once (and only once) at every sample.
 */
class SawToothOsc : public Phasor
{
    float output (float _phase) override
    {
        return (_phase * 2.0f) - 1.0f;
    }
};

//==============================================================================
/**
 Creates an anti aliased sawtooth oscillator instance, following the paper by Vesa Valimaki and Antti Huovilainen
 https:ieeexplore.ieee.org/abstract/document/4117934?casa_token=tbLyqzTQiQMAAAAA:beJiY1lmePb8wotluILgs224rkZMzD65fEWs3robfxUCFua8ib8VbO3KCVIPImqufl4QCL-6cA
 The process() method must be run once (and only once) at every sample.
 
 Uses the Polyblep function from the CustomFunction.h file
 */
class AntiAliasSawToothOsc : public Phasor
{
    float output (float _phase) override
    {
        return (_phase * 2.0f) - 1.0f - polyblep(_phase, getPhaseDelta());
    }
    
    /// Determines by how much a sample must be changed when dealing with the AntiAliasedSawTooth oscillator found in the Osciilator.h file.
    float polyblep(float _phase, float _phaseDelta)
    {
        if (_phase > 1.0f - _phaseDelta)
        {
            float t = (_phase - 1.0) / _phaseDelta;
            return t*t + t+t + 1.0;
        }
        
        else if (_phase < _phaseDelta)
        {
            float t = _phase / _phaseDelta;
            return -t*t + t+t - 1.0;
        }
        
        else
        {
            return 0.0f;
        }
    }

};



