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
    
    GrainBuffer grainBuffer;
    float maxDelaySizeInSeconds = 5.0f;
    float delaySizeInSeconds = 1.0f;
    int sampleRate;
    
    GrainManager grainManager;
    
    std::vector<Grain> grains;
    int maxGrainCount = 5;
};
