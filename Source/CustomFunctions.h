/*
  ==============================================================================

    CustomFunctions.h
    Created: 13 Apr 2022 11:06:05pm
    Author:  B162025

  ==============================================================================
*/

#pragma

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
