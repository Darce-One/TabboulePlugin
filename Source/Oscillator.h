//
//  Oscillator.h test
//  Functions and classes challenges
//
//  Created by Andreas Papaeracleous on 28/02/2022.
//

#ifndef Oscillator_h
#define Oscillator_h
#include <cmath>


/**
 Creates a Phasor instance.
 It takes in a frequency and a sample rate and returns a phase between 0 and 1.
 The process() method must be run once (and only once) at every sample.
 
 Used as a base class for many oscillator types, including Sine, Square, Triangle and Saw.
 */
class Phasor
{
private:
    float sampleRate = 44100.0f;
    float phase = 0.0f;
    float phaseDelta;
    float frequency;
    bool newCycle = true;
    
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
};

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







/// Creates an oscillator instance
class Oscillator
{
private:
    float sampleRate = 44100.0f;
    float phase = 0.0f;
    float phaseDelta;
    float frequency;
    const float pi = 3.14159265359;
    
public:
    
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
    float processSine()
    {
        ///Process the next sample of the sinewave oscillator:
        // Keep phase in range 0-1
        if (phase > 1.0f)
            phase -= 1.0f;
        
        // Calculate output sample
        float output = 0.5f * sin (2 * pi * phase);
        
        // Update Phase
        phase += phaseDelta;
        return output;
    }
    float processSquare()
    {
        ///Process the next sample of the Squarewave oscillator:
        // Keep the phase in range 0-1
        if (phase > 1.0f)
            phase -= 1.0f;
        
        // Calculate output sample
        float output = 0.5f * tanhf (30.0f * sin (2.0f * pi * phase));
        
        // Update phase
        phase += phaseDelta;
        return output;
    }
    float processTriangle()
    {
        ///Process the next samples of the Trianglewave Oscillator
        // Keep the phase in range 0-1
        if(phase > 1.0f)
            phase -= 1.0f;
        
        float output = 2.0f * (fabs (phase - 0.5f) - 0.25f);
        
        // Update phase
        phase += phaseDelta;
        return output;
    }
    float processSawtooth()
    {
        ///Process the next samples of the Sawtooth wave Oscillator
        // Keep the phase in range 0-1
        if(phase > 1.0f)
            phase -= 1.0f;
        
        float output = phase - 0.5f;
        return output;
    }
    
};

#endif /* Oscillator_h */
