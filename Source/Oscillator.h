/*
  ==============================================================================

    Oscillator.h
    Created: 16 Apr 2022 5:42:07pm
    Author:  B162025

  ==============================================================================
*/

#pragma once
#include <cmath>


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
    
    
    void setSampleRate (float _sampleRate)
    {
        ///sets the sample rate of the oscillator
        sampleRate = _sampleRate;
    }
    
    void setFrequency (float _frequency)
    {
        ///sets the frequency of the oscillator
        frequency = _frequency;
        phaseDelta = frequency / sampleRate;
    }
    void setPhase (float p)
    {
        ///resets the phase of the phasor
        phase = p;
    }
    
    /// Returns the phase of the oscillator. Useful for getting more options out of one Oscillator instance.
    float getPhase()
    {
        return phase;
    }
    
    float process()
    {
        ///Process the next sample of the sinewave oscillator:
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
    
    virtual float output (float _phase)
    {
        ///Determines what happens to the phase
        ///In the phasor class, it doesn't alter it
        return _phase;
    }
    
    bool newCycleStarted()
    {
        ///returns true at every new phase cycle;
        return newCycle;
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
    ///Creates a square oscillator instance
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
    ///Creates a sawtooth oscillator instance
    float output (float _phase) override
    {
        return (_phase * 2.0f) - 1.0f;
    }
};
