/*
  ==============================================================================

    FFTSynth.h
    Created: 23 Apr 2022 10:05:50am
    Author:  B162025

  ==============================================================================
*/

#pragma once

/**
 This class creates an instance of a synth which listens to an input and follows it.
 It is designed to be used in conjunction with the grainBuffer, Grain and Oscillator classes.
 
 A lot of the code found here is taken from the JUCE tutorial on the Fast Fourier Transform, which can be found here:
 https://docs.juce.com/master/tutorial_simple_fft.html
 
 Variable names and types are identical to those used in that tutorial, it is the member functions that change to adapt to this use.
 
 The FFT has an order of 16 to ensure that any incoming data from the grains will fit.
 */
class FFTSynth
{
public:
    
    /// Constructor
    FFTSynth() : forwardFFT (fftOrder)
    {
        std::fill (fftData.begin(), fftData.end(), 0.0f);
        std::fill (fifo.begin(), fifo.end(), 0.0f);
    }
    
    /// Method that takes in the grain data, sample by sample. Must be called before other methods.
    void writeInSamples (float leftSample, float rightSample, bool newGrainStarted)
    {
        if (newGrainStarted == true)
        {
            std::fill (fftData.begin(), fftData.end(), 0.0f);
            std::copy (fifo.begin(), fifo.end(), fftData.begin());
            std::fill (fifo.begin(), fifo.end(), 0.0f);
            fifoIndex = 0;
            processFFT();
        }
        
        nextFFTBlockReady = newGrainStarted;
        float monoSample = (leftSample + rightSample) * 0.5f;
        fifo[(size_t) fifoIndex++] = monoSample;
    }
    
    
    void processFFT()
    {
        forwardFFT.performFrequencyOnlyForwardTransform (fftData.data());
    }
    
    
    
    
    static constexpr auto fftOrder = 16;                // Order 16 --> 2 ^ 16 = 65532 samples covers grains of 500 ms at 92 kHz
    static constexpr auto fftSize = 1 << fftOrder;
    
private:
    juce::dsp::FFT forwardFFT;                          // fft instance
    std::array<float, fftSize> fifo;                    // input array
    std::array<float, fftSize * 2> fftData;             // transform data
    int fifoIndex = 0;                                  // temporary index keeps track of filled in samples
    bool nextFFTBlockReady = false;                     // trigger for fft operation
    
};
