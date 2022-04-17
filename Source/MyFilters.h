/*
  ==============================================================================

    MyFilters.h
    Created: 12 Mar 2022 10:40:55am
    Author:  B162025

  ==============================================================================
*/

#pragma once
#include <cmath>
#include "Oscillator.h"

/**
 Creates a Filter instance.
 As of yet there is only a Low Pass Filter Process.
 The filter design is very basic.
 */
class MyFilter
{
public:
    
    //Constructor
    MyFilter (float _sampleRate, float _cutoffFrequency, float _Qres = 1.0)
    {
        ///Takes in the sample rate, cutoff frequency and resonnance parameter [1-20]
        sampleRate = _sampleRate;
        cutoffFrequency = _cutoffFrequency;
        Qres = _Qres;
        recalibrateFilter();
    }
    
    //Destructor
    ~MyFilter()
    {
    }
    
    
    void recalibrateFilter()
    {
        ///resets the parameters of the filter
        if (Qres <= 1)
            Qres = 1.0f;
        
        thetaCutoff = (2 * pi * cutoffFrequency) / sampleRate;
        d = 1.0/Qres;
        beta = 0.5f * (1 - ((d/2.0f) * sin (thetaCutoff))) / (1 + ((d / 2.0f) * sin (thetaCutoff)));
        gamma = (0.5 + beta) * cos (thetaCutoff);
        a0 = (0.5 + beta - gamma) / 2.0;
        a1 = 2.0 * a0;
        a2 = a0;
        b1 = -2.0 * gamma;
        b2 = 2 * beta;
    }
    
    //set cutoff frequency
    void setCutoffFrequency (float _cutoffFrequency)
    {
        ///sets the cutoff frequency
        cutoffFrequency = _cutoffFrequency;
        recalibrateFilter();
    }
    
    // set the resonnance parameter Q
    void setQres (float _Qres)
    {
        ///sets the resonnance parameter [1-20]
        Qres = _Qres;
        recalibrateFilter();
    }
    
    //set both the res and cutoff parameters
    void setParameters (float _cutoffFrequency, float _Qres)
    {
        ///sets both the resonnance and cutoff parameters
        cutoffFrequency = _cutoffFrequency;
        Qres = _Qres;
        recalibrateFilter();
    }
    
    //set the sample rate
    void setSampleRate (float _sampleRate)
    {
        ///sets the sample rate
        sampleRate = _sampleRate;
        recalibrateFilter();
    }
    
    //process sample LPF
    float processLPFSample (float inputSample)
    {
        ///Takes in the current sample and outputs the filtered sample
        //calculate output sample
        float outputSample = d0 * inputSample + c0 * (a0 * inputSample +
                                                      a1 * x1 +
                                                      a2 * x2 -
                                                      b1 * y1 -
                                                      b2 * y2);
        //store past samples
        x2 = x1;
        x1 = inputSample;
        y2 = y1;
        y1 = outputSample;
        
        //return output
        return outputSample;
    }
    
    //set mix
    void setDryWet (float dry, float wet)
    {
        ///Choose the mix parameters
        d0 = dry;
        c0 = wet;
    }
    
    //==========================================================================
private:
    float cutoffFrequency;              // cutoff frequency
    float Qres;                         // Resonance parameter range (1-20)
    int sampleRate;                     // sample rate
    const float pi = 3.14159265359;     // value of pi
    float thetaCutoff;                  // useable cutoff variable
    float d;                            // inverse of Q
    float beta;                         // value used in calculations
    float gamma;                        // value used in calculations
    
    //filter coefficients
    float a0, a1, a2, b1, b2;
    float c0 = 1.0;
    float d0 = 0.0;
    
    //past samples
    float x1 = 0.0f;
    float x2 = 0.0f;
    float y1 = 0.0f;
    float y2 = 0.0f;
};


/**
 Creates a Function (envelope) instance.
 Requires a sample rate, a ramp time, and a trigger.
 As of yet, there is only a sine ramp option, and it must be called every sample.
 */
class FunctionGenerator
{
public:
    
    //Constructor
    FunctionGenerator (float _sampleRate, float _rampTime)
    {
        ///Takes in the sample rate and the ramp time (in seconds).
        sinOsc.setSampleRate (_sampleRate);
        sinOsc.setFrequency (1.0f / (2.0f * _rampTime));
    }
    
    float sineRamp (bool trigger)
    {
        ///Takes a bool trigger, returns a coefficient between 0-1.
        
        // Look for a trigger to start the envelope.
        if (trigger == true)
        {
            isRunning = true;
            sinOsc.setPhase(0.5f);
            sinOutCoeff = 1.0f;
        }
        
        // Stops the envelope when it's over.
        if (sinOsc.newCycleStarted() == true && isRunning == true)
        {
            isRunning = false;
            sinOutCoeff = 0.0f;
        }
        
        return -1.0f * sinOutCoeff * sinOsc.process();          // returns in the range 0-1
    }
    
    void setSineRampTime (float TimeInSeconds)
    {
        ///sets the total ramp time (rise and fall)
        sinOsc.setFrequency (1.0f / (2.0f * TimeInSeconds));
    }
    
    bool getIsRunning()
    {
        ///check if envelope is active
        return isRunning;
    }
    
    
private:
    bool isRunning = false;
    SineOsc sinOsc;
    float sinOutCoeff = 0;
};
