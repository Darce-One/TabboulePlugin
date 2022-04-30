/*
  ==============================================================================

    FFTSynth.h
    Created: 23 Apr 2022 10:05:50am
    Author:  B162025

  ==============================================================================
*/

#pragma once
#include "CustomFunctions.h"

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
    
    /**
     Constructor for class.
     
     Takes in the sample rate, the envelope shape and the grain length in seconds.
     
     It starts by clearing the input - output float buffers used in the fourier transform, sets the sample rate,
     sets the frequency of 25Hz to the Hann Oscillator (for a half period of 0.02 seconds),
     sets the value of the float to be multiplied by the returned index of the highest,
     sets the filter coefficients to remove lows and highs from incoming audio.
     
     @param _sampleRate Sample rate of project.
     @param _envelopeShape Float between [0-1], 0 denoting short attack and long release, and 1 denoting long attack short release.
     @param _grainLenthInSeconds length of incoming grains in seconds, to make the outgoing grains the same length
     */
    FFTSynth(int _sampleRate, float _envelopeShape, float _grainLengthInSeconds, float _precision, float _freqA) : forwardFFT (fftOrder)
    {
        std::fill (fftData.begin(), fftData.end(), 0.0f);
        std::fill (fifo.begin(), fifo.end(), 0.0f);
        
        sampleRate = _sampleRate;
        triOsc.setSampleRate(sampleRate);
        sinOscForHann.setSampleRate(sampleRate);
        
        sinOscForHann.setFrequency(25.0f);
        
        indexMultiplierForFrequencyAquisition = sampleRate / float(fftSize);
        
        setEnvelopeParams(_envelopeShape, _grainLengthInSeconds);
        setRealEnvelopeParams();
        
        lpFilter.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, 5000.0f));
        hpFilter.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, 60.0f));
        
        setPrecision(_precision, _freqA);
    }
    
    /**
     Method to be run at each sample.
     
     Takes in the incoming samples and runs the algorithm.
     
     @param leftSample the left sample of the grain
     @param rightSample the right sample of the grain
     @param newGrainStarted boolean to be taken from the grain class method .newGrainStarted.
     
     First, method checks if the information coming in corresponds to a new grain.
     If so, it refreshed the buffers with 0s while moving the old grain samples into the fft buffer for potential analysis.
     It then ticks the box saying it's listening, and the hann window is reset.
     The last fft is only analysed if the last grain was loud enough. This saves on computation, and avoids unnecessary grain playing when audio playback is null.
     It then resets the maxSample value back to 0.
     
     
     After that, it checks whether is should be listenning for samples.
     If it should, it sums the channels to Mono,  applies a hann window to them and stores them in the fifo sample by sample.
     While doing that, it keeps track of the maximum amplitude used in the signal to determine whether or not to process the FFT.
     It stops listenning once the hann window has ended.
     
     */
    void writeInSamples (float leftSample, float rightSample, bool newGrainStarted, float newThreshold, float _chanceToSkip, float _stereoRandomness)
    {
        // When a new grain starts: refresh buffers, enable listenning, and depending on the whether the last grain was loud enough, analyse it.
        if (newGrainStarted == true)
        {
            // Refreshing buffers
            std::fill (fftData.begin(), fftData.end(), 0.0f);
            std::copy (fifo.begin(), fifo.end(), fftData.begin());
            std::fill (fifo.begin(), fifo.end(), 0.0f);
            fifoIndex = 0;
            
            // enable listenning
            listenning = true;
            
            // Resetting hann window phase
            sinOscForHann.setPhase(0.0f);
            
            // Check if last grain was loud enough
            if (grainMaxAbsSample > grainMaxAbsSampleThreshold && random.nextFloat() > _chanceToSkip)
            {
                synthVolume = grainMaxAbsSample;
                processFFT();
                stereoVolumeLeft = 0.5f + ((random.nextFloat() - 0.5f) * _stereoRandomness);
                stereoVolumeRight = 1.0f - stereoVolumeLeft;
            }
            // Reset the last max abs sample
            grainMaxAbsSample = 0.0f;
            
            // Reset the filters
            lpFilter.reset();
            hpFilter.reset();
        }
        
        // While listenning (1 hann window length): store windowed incoming audio in fifo, keep track of max sample. stop listenning at the end of the hann window.
        if (listenning == true)
        {
            // Window and filter audio
            float hannToBeSquared = sinOscForHann.process();
            float monoSampleRaw = (leftSample + rightSample) * 0.5f * hannToBeSquared * hannToBeSquared;
            float monoSample = lpFilter.processSingleSampleRaw(hpFilter.processSingleSampleRaw(monoSampleRaw));
            
            // Store in fifo
            fifo[(size_t) fifoIndex++] = monoSample;
            
            // Keep track of max sample
            float AbsSample = std::abs(monoSample);
            
            if (AbsSample > grainMaxAbsSample)
                grainMaxAbsSample = AbsSample;
            
            // Check when to stop listenning
            if (sinOscForHann.getPhase() > 0.5)
                listenning = false;
        }
        
        setGrainMaxAbsSampleThreshold (newThreshold);
    }
    
    
    
    /**
     Stores temporary values for envelope shape and grain length, which will be used only when needed (Hence the private method setRealEnvelopeParams)
     
     @param _envelopeShape Float between [0-1], 0 denoting short attack and long release, and 1 denoting long attack short release.
     @param _grainLenthInSeconds length of incoming grains in seconds, to make the outgoing grains the same length
     */
    void setEnvelopeParams(float _envelopeShape, float _grainLengthInSeconds)
    {
        envelopeShapeTemp = _envelopeShape;
        grainLengthInSamplesTemp = _grainLengthInSeconds * sampleRate;
    }
    
    
    /**
     Method to also be called every sample.
     */
    float processSynth()
    {
        if (synthIsPlaying)
        {
            sampleCount += 1;
            
            if (sampleCount < grainLengthInSamples)
            {
                float synthSample = triOsc.process();
                
                if (sampleCount < envelopeShapeInSamples)
                {
                    return synthSample * synthVolume * 2.0f * sampleCount / float (grainLengthInSamples);
                }
                
                else
                {
                    return synthSample * synthVolume * (sampleCount * descentSlope + descentIntercept);
                }
            }
            else
            {
                synthIsPlaying = false;
                return 0.0f;
            }
        }
        else
        {
            return 0.0f;
        }
    }
    
    void setGrainMaxAbsSampleThreshold(float newThreshold)
    {
        grainMaxAbsSampleThreshold = newThreshold;
    }
    
    float getStereoVolumeLeft()
    {
        return stereoVolumeLeft;
    }
    
    float getStereoVolumeRight()
    {
        return stereoVolumeRight;
    }
    
    void setPrecision(float _precision, float _freqA = 440.0f)
    {
        precision = _precision;
        freqA = _freqA;
    }
    
    
    static constexpr auto fftOrder = 16;                // Order 16 --> 2 ^ 16 = 65532 samples covers grains of 500 ms at 92 kHz
    static constexpr auto fftSize = 1 << fftOrder;
    
private:
    juce::dsp::FFT forwardFFT;                          // fft instance
    std::array<float, fftSize> fifo;                    // input array
    std::array<float, fftSize * 2> fftData;             // transform data
    int fifoIndex = 0;                                  // temporary index keeps track of filled in samples
//    bool nextFFTBlockReady = false;                     // trigger for fft operation
    bool listenning = false;                            // status of hann window.
    int sampleRate;                                     // Sample rate of project
//    SineOsc sinOsc;
    TriOsc triOsc;
//    SoftSquareOsc sqOsc;
    SineOsc sinOscForHann;
    float grainMaxAbsSample = 0.0f;
    float grainMaxAbsSampleThreshold = 0.01f;
    float synthFrequency = 1.0f;
    float indexMultiplierForFrequencyAquisition;
    float envelopeShape;
    int envelopeShapeInSamples;
    int grainLengthInSamples;
    float envelopeShapeTemp;
    int grainLengthInSamplesTemp;
    int sampleCount = 0;
    bool synthIsPlaying = false;                        
    float descentSlope;
    float descentIntercept;
    float synthVolume = 1.0f;
    juce::IIRFilter lpFilter;
    juce::IIRFilter hpFilter;
    juce::Random random;
    float stereoVolumeLeft = 0.5f;
    float stereoVolumeRight = 0.5f;
    float freqA = 440.0f;
    float precision = 0.2f;
    
    /**
     sets the temporary values of envelope shape and grain size in stone when called by the private method processFFT()
     
     Calculates parameters for the synth envelope.
     */
    void setRealEnvelopeParams()
    {
        envelopeShape = envelopeShapeTemp;
        grainLengthInSamples = grainLengthInSamplesTemp;
        envelopeShapeInSamples = envelopeShape * grainLengthInSamples;
        descentSlope = 1.0f / ((envelopeShape - 1.0f) * grainLengthInSamples);
        descentIntercept = -1.0f * descentSlope * grainLengthInSamples;
    }
    
    
    /**
     Private method runs the FFT on the last grain and applies it accordingly:
     
     From the FFT, grab the peak frequency simply by finding the index of the highest sample in the first half of the transform,
     and multiplying it by the ratio sampleRate / FFT size.
     
     Set the calculated frequency to the synth, recalibrate envelope parameters, reset the sample count to 0.
     
     */
    void processFFT()
    {
        forwardFFT.performFrequencyOnlyForwardTransform (fftData.data());
        
        // Get Main Frequency
        int index = 0;
        float maxAmplitude = 0.0f;
        for (int i=0; i<fftSize; i++)
        {
            float binAbsAmplitude = std::abs(fftData[i]);
            if (binAbsAmplitude > maxAmplitude)
            {
                index = i;
                maxAmplitude = binAbsAmplitude;
            }
        }
        
        synthFrequency = index * indexMultiplierForFrequencyAquisition;
        
        
        //set it to the synth
        triOsc.setFrequency (adjustedFrequency (synthFrequency, precision, freqA));

        // trigger the synth
        sampleCount = -1;               // Starts at -1 (not 0) because the += 1 happens at the start of the loop.
        setRealEnvelopeParams();
        synthIsPlaying = true;
        triOsc.setPhase(0.25f);
    }
};
