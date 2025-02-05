/*
  ==============================================================================

    CustomFunctions.h
    Created: 13 Apr 2022 11:06:05pm
    Author:  B162025

  ==============================================================================
*/

#pragma once

#include <math.h>
#include "Oscillator.h"

/// Sets mapped reverb parameters according to Two custom parameters.
void setReverbParams(juce::Reverb::Parameters& reverbParams, float oilLevel, float spiceLevel)
{
    //Calculate Params
    float wetLevel = 0.8f * oilLevel;
    float dryLevel = 1.0f - wetLevel;
    float roomSize = 0.5f + (0.3f * (oilLevel * oilLevel));
    
    //Implement Params
    reverbParams.width = spiceLevel;
    reverbParams.dryLevel = dryLevel;
    reverbParams.wetLevel = wetLevel;
    reverbParams.roomSize = roomSize;
}

///Converts Midi note (0-127) to float frequency
float midiToFrequency (int midiNote, float freqA = 440.0f)
{
    return freqA * pow (2.0, (midiNote - 69) / 12.0);
}

///Converts frequency into midi note
float frequencyToMidi (float frequency, float freqA = 440.0f)
{
    return 12 * log2 (frequency / freqA) + 69;
}

/// Returns the tuned frequency according to precision and tuning.
float adjustedFrequency (float frequency, float precision, float freqA = 440.0f)
{
    float relativeMidiNote = frequencyToMidi (frequency, freqA);
    float nearestMidiNote = round (relativeMidiNote);
    float adjustedMidi = ((1 - precision) * relativeMidiNote) + (precision * nearestMidiNote);
    
    return midiToFrequency (adjustedMidi, freqA);
}


/**
 Processes three oscillators and returns the mixed output of all three according to the parameter.
 @param OscillatorSelect float in range [1-3]
 */
float processOscillators(float oscillatorSelect, SineOsc& _sineOsc, TriOsc& _triOsc, AntiAliasSawToothOsc& _sawOsc)
{
    float sinSample = _sineOsc.process();
    float triSample = _triOsc.process();
    float sawSample = _sawOsc.process();
    
    float sinVolume = -0.2f * oscillatorSelect + 1.5f;
    float triVolume = 1.0f - fabs (oscillatorSelect - 2.0f);
    float sawVolume = 0.5 * (0.5f * oscillatorSelect - 0.5f);
    
    return 0.33f * ((sinSample * sinVolume) + (triSample * triVolume) + (sawSample * sawVolume));
}


