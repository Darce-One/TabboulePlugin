/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TabboulehAudioProcessor::TabboulehAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
parameters(*this, nullptr, "ParameterTree", {
    
    // Parameters
    std::make_unique<juce::AudioParameterFloat>("buffer_Size" ,"Bowl Size", 1.0f, 4.99f, 2.0f),
    std::make_unique<juce::AudioParameterFloat>("grain_Randomisation" ,"Mama's Hands", 0.0f, 1.0f, 0.3f),
    std::make_unique<juce::AudioParameterFloat>("grain_Shape" ,"Parsley Shape", 0.0f, 1.0f, 0.6f),
    std::make_unique<juce::AudioParameterFloat>("grain_Length" ,"Chop Size", 0.020f, 0.5f, 0.1f),
    std::make_unique<juce::AudioParameterFloat>("active_Grains" ,"Onion", 1.0f, 4.99f, 2.0f),
    std::make_unique<juce::AudioParameterFloat>("chanceToSkip_Grain" ,"Bourghol", 0.0f, 1.0f, 0.05f),
    std::make_unique<juce::AudioParameterFloat>("grain_StereoRandomness" ,"Spices", 0.0f, 1.0f, 0.2f),
    std::make_unique<juce::AudioParameterFloat>("synth_Volume" ,"Tomato Size", 0.0f, 1.0f, 0.2f),
    std::make_unique<juce::AudioParameterFloat>("synth_Envelope" ,"Tomato Cut", 0.01f, 0.99f, 0.1f),
    std::make_unique<juce::AudioParameterFloat>("synth_Volume_Threshold" ,"Tomato Age", juce::NormalisableRange<float>(0.01f, 0.90f, 0.01f, 0.35), 0.2f),
    std::make_unique<juce::AudioParameterFloat>("frequency_Precision" ,"Mint", 0.0f, 1.0f, 0.6f),
    std::make_unique<juce::AudioParameterFloat>("highPass_Frequency" ,"Lemon", juce::NormalisableRange<float>(20.0f, 2500.f, 1.0f, 0.3), 100.0f),
    std::make_unique<juce::AudioParameterFloat>("reverb_Amount" ,"Oil", 0.0f, 0.99f, 0.4f),
    std::make_unique<juce::AudioParameterFloat>("freqA" ,"Tuning: A = (Hz)", 400.0f, 500.0f, 440.0f),


})
{
    // Parameters
    bufferSizeParam = parameters.getRawParameterValue("buffer_Size");
    grainRandomisationParam = parameters.getRawParameterValue("grain_Randomisation");
    grainShapeParam = parameters.getRawParameterValue("grain_Shape");
    grainLengthParam = parameters.getRawParameterValue("grain_Length");
    activeGrainsParam = parameters.getRawParameterValue("active_Grains");
    chanceToSkipGrainParam = parameters.getRawParameterValue("chanceToSkip_Grain");
    grainStereoRandomnessParam = parameters.getRawParameterValue("grain_StereoRandomness");
    synthVolumeParam = parameters.getRawParameterValue("synth_Volume");
    synthEnvelopeShapeParam = parameters.getRawParameterValue("synth_Envelope");
    synthVolumeThresholdParam = parameters.getRawParameterValue("synth_Volume_Threshold");
    frequencyPrecisionParam = parameters.getRawParameterValue("frequency_Precision");
    hpFrequencyParam = parameters.getRawParameterValue("highPass_Frequency");
    reverbAmountParam = parameters.getRawParameterValue("reverb_Amount");
    freqAParam = parameters.getRawParameterValue("freqA");
}

TabboulehAudioProcessor::~TabboulehAudioProcessor()
{
}

//==============================================================================
void TabboulehAudioProcessor::prepareToPlay (double _sampleRate, int samplesPerBlock)
{
    // Store sampleRate for later use:
    sampleRate = _sampleRate;
    
    // Initialise the Grain Buffer instance:
    grainBuffer.initialise (maxDelaySizeInSeconds, _sampleRate);
    grainBuffer.setBufferSize (*bufferSizeParam);

    // Initialise the grain manager:
    grainManager.managePhases(*activeGrainsParam);
    
    // Initialise the grain intances:
    for (int i=0; i<maxGrainCount; i++)
    {
        if (grains.size() < maxGrainCount)
            grains.push_back(Grain(_sampleRate, grainManager.getPhaseForGrain(i), *grainLengthParam));
    }
    
        //Initialise the FFTSynth instances:
        for (int i=0; i<maxFftSynthCount; i++)
        {
            if (fftsynths.size() < maxFftSynthCount)
                fftsynths.push_back(FFTSynth(_sampleRate, 0.5f, *grainLengthParam, *frequencyPrecisionParam, *freqAParam));
        }
    
    // Initialise the filters and reverb:
    hpFilterL.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, *hpFrequencyParam));
    hpFilterR.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, *hpFrequencyParam));
    
    setReverbParams(reverbParams, *reverbAmountParam, *grainStereoRandomnessParam);
    reverb.setSampleRate(sampleRate);
    reverb.setParameters(reverbParams);
    
}

void TabboulehAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get read pointers:
    auto* inputLeftChannelData = buffer.getReadPointer(0);
    auto* inputRightChannelData = buffer.getReadPointer(1);
    
    // Get write pointers:
    auto* outputLeftChannelData = buffer.getWritePointer(0);
    auto* outputRightChannelData = buffer.getWritePointer(1);
    
    // Check if activeGrainsParam (Onion) changed:
    if (*activeGrainsParam != activeGrains)
    {
        // If true, recalculate the phases of the grains and set them to each grain:
        activeGrains = *activeGrainsParam;
        grainManager.managePhases(activeGrains);
        
        for (int i=0; i<maxGrainCount; i++)
            grains[i].setGrainPhase(grainManager.getPhaseForGrain(i));

    }
    
    // Set the tuning precision to the FFTSynths:
    for (int i=0; i<maxGrainCount; i++)
        fftsynths[i].setPrecision (*frequencyPrecisionParam, *freqAParam);
    
    
    // Recalibrate the High Pass Filters to user setting:
    hpFilterL.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, *hpFrequencyParam));
    hpFilterR.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, *hpFrequencyParam));
    
    //DSP Loop
    for (int DSPiterator = 0; DSPiterator < buffer.getNumSamples(); DSPiterator++)
    {
        // Get the filtered incoming audio samples for both L and R channels:
        float inputSampleLeft = hpFilterL.processSingleSampleRaw(inputLeftChannelData[DSPiterator]);
        float inputSampleRight = hpFilterR.processSingleSampleRaw(inputRightChannelData[DSPiterator]);
        
        // Store the collected samples into the buffer, and check for size:
        grainBuffer.writeVal(inputSampleLeft, inputSampleRight);
        grainBuffer.setBufferSize(*bufferSizeParam);
        
        // Initialise out samples:
        float outSampleLeft = 0.0f;
        float outSampleRight = 0.0f;
        
        // Following operations done at a grain level:
        for (int i=0; i<maxGrainCount; i++)
        {
            // Process the ith grain:
            grains[i].process(*grainLengthParam,
                              grainBuffer.getMaxReadPos(),
                              *grainRandomisationParam,
                              *grainShapeParam,
                              *chanceToSkipGrainParam,
                              *grainStereoRandomnessParam);
            
            // Get the right and left out samples from active grains (0 for inactive):
            float unprocessedGrainSampleL = grainBuffer.readValL(grains[i].getReadPos()) * grainManager.getVolumeForGrain(i);
            float unprocessedGrainSampleR = grainBuffer.readValR(grains[i].getReadPos()) * grainManager.getVolumeForGrain(i);
            
            // Write in L/R Samples into FFT buffers and process the instance.
            fftsynths[i].writeInSamples(unprocessedGrainSampleL,
                                        unprocessedGrainSampleR,
                                        grains[i].newGrainStarted(),
                                        *synthVolumeThresholdParam,
                                        *chanceToSkipGrainParam,
                                        *grainStereoRandomnessParam);
            
            // Set envelope parameters in synths:
            fftsynths[i].setEnvelopeParams(*synthEnvelopeShapeParam, *grainLengthParam);
            
            // Get the output of th synth:
            float synthOut = fftsynths[i].processSynth() * *synthVolumeParam;
            
            // Calculate the Left sample:
            float outGrainSampleL = (((2.0f/float(*activeGrainsParam))
                                    * unprocessedGrainSampleL
                                    * grains[i].getStereoVolumeLeft()))
                                    + (synthOut * fftsynths[i].getStereoVolumeLeft());

            // Calculate the Right sample:
            float outGrainSampleR = (((2.0f/float(*activeGrainsParam))
                                    * unprocessedGrainSampleR
                                    * grains[i].getStereoVolumeRight()))
                                    + (synthOut * fftsynths[i].getStereoVolumeRight());
            
            // Add the L/R samples to the main output:
            outSampleLeft  += outGrainSampleL;
            outSampleRight += outGrainSampleR;
        }
        
        // Write samples to output:
        outputLeftChannelData[DSPiterator]  = outSampleLeft;
        outputRightChannelData[DSPiterator] = outSampleRight;
        
        // Update Reverb Parameters:
        setReverbParams(reverbParams, *reverbAmountParam, *grainStereoRandomnessParam);
        reverb.setParameters(reverbParams);
        
        //=============
        // HERE ONLY FOR TESTING, zone for breakpoint if necessary! DELETE WHEN DONE!
//        testInt++;
//        if (testInt > 6000)
//        {
//            testInt = 0;
//        }
        //=============
    }
    
    // Apply reverb to buffer
    reverb.processStereo(outputLeftChannelData, outputRightChannelData, buffer.getNumSamples());
}

void TabboulehAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TabboulehAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


//==============================================================================
const juce::String TabboulehAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TabboulehAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TabboulehAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TabboulehAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TabboulehAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TabboulehAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TabboulehAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TabboulehAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TabboulehAudioProcessor::getProgramName (int index)
{
    return {};
}

void TabboulehAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}


//==============================================================================
bool TabboulehAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TabboulehAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void TabboulehAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    // getStateInformation
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void TabboulehAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    // setStateInformation
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TabboulehAudioProcessor();
}
