/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GrainBuffer.h"
#include "CustomFunctions.h"
#include "Grain.h"
#include "MyFilters.h"
#include "FFTSynth.h"
#include <vector>

//==============================================================================
/**
*/
class TabboulehAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TabboulehAudioProcessor();
    ~TabboulehAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabboulehAudioProcessor)
    
    // Audio processor value tree state definition for user interface and preset saving
    juce::AudioProcessorValueTreeState parameters;
        
    GrainBuffer grainBuffer;
    float maxDelaySizeInSeconds = 5.0f;
    std::atomic<float>* bufferSizeParam;
    float bufferSize = 1.0f;
    
    std::atomic<float>* chanceToSkipGrainParam;
    float chanceToSkipGrain = 0.2f;
    
    std::atomic<float>* grainStereoRandomnessParam;
    float grainStereoRandomness = 0.2f;
    
    GrainManager grainManager;
    
    std::vector<Grain> grains;
    int maxGrainCount = 5;
    
    std::vector<FFTSynth> fftsynths;
    int maxFftSynthCount = maxGrainCount;
    
    std::atomic<float>* activeGrainsParam;
    float activeGrains = 2.8;
    std::atomic<float>* grainLengthParam;
    float grainLength = 0.05;
    
    std::atomic<float>* grainRandomisationParam;
    float grainRandomisation = 0.5;
    std::atomic<float>* grainShapeParam;
    float grainShape = 0.5;
    
    float synthVolume = 0.8;
    float synthEnvelopeShape = 0.5;
    
    float testInt = 0;
    
    
};
