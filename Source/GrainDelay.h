/*
  ==============================================================================

    GrainDelay.h
    Created: 13 Apr 2022 11:05:09pm
    Author:  B162025

  ==============================================================================
*/

#pragma once

class GrainDelay
{
public:

    /**
     acts as but isn't a constructor for GrainDelay class
     
     @param _maxDelayTime maximum length of buffers in seconds
     @param _sampleRate sample rate
     */
    void initialise(int _maxDelayTime, int _sampleRate)
    {
        sampleRate = _sampleRate;
        maxSize = _maxDelayTime * _sampleRate;
        bufferL = new float[maxSize];
        bufferR = new float[maxSize];
        
        for (int i=0; i<maxSize; i++)
        {
            bufferL[i] = 0.0f;
            bufferR[i] = 0.0f;
        }
    }
    
    ///Destructor
    ~GrainDelay()
    {
        // If bufferL exists, delete it!
        if (bufferL) delete[] bufferL;
        if (bufferR) delete[] bufferR;
    }
    
    
    /**
     Sets the size of the buffer
     
     @param _currentSize the new size of the buffers
     */
    void setBufferSize(float _currentSize)
    {
        currentSizeTemporary = floor (_currentSize * sampleRate);
    }
    
    
    /**
     Writes in the Left and Right samples into their buffers
     
     @param inputSampleL the Left input sample
     @param inputSampleR the Right input sample
     */
    void writeVal(float inputSampleL, float inputSampleR)
    {
        if (writePos >= currentWriteSize || writePos >= maxSize)
        {
            writePos = 0;
            // Must insure that the current size used here only changes when the
            // write position is reset to ensure no gaps in playback
            oldWriteSizeTemporary = currentWriteSize;
            currentWriteSize = currentSizeTemporary;
            
            if (currentWriteSize == oldWriteSizeTemporary)
                oldWriteSize = currentWriteSize;
        }
        
        bufferL[writePos] = inputSampleL;
        bufferR[writePos] = inputSampleR;
        writePos++;
    }
    
    /**
     Reads the LEFT sample at the reading position of the specified grain.
     
     This function also increments the readPos as necessary, and therefore should be called BEFORE readValR().
     
     @param index The reading position index [0-4]
     @return sample value at reading position
     */
    float readValL(int index)
    {
        if (readPos[index] >= oldWriteSize || readPos[index] >= maxSize)
        {
            readPos[index] = 0;
            // Must ensure that the current size used here (oldWriteSize) is the old write size as to ensure that
            // the samples at the turnaround follow each other. We must also make sure that the old write size is
            // not overwritten
            oldWriteSize = oldWriteSizeTemporary;
        }
        
        return bufferL[readPos[index]];
        readPos[index]++;
    }
    
    /**
     Reads the RIGHT sample at the reading position of the specified grain.
     
     This function does not increment the readPos, and must be called AFTER readValL().
     
     @param index The reading position index [0-4]
     @return sample value at reading position
     */
    float readValR(int index)
    {
        return bufferR[readPos[index]];
        readPos[index]++;
    }
    
    
    float processGrain(int index, float grainSizeInSeconds)
    {
        if(!grainIsPlaying[index])
        {
            grainIsPlaying[index] = true;
            int grainSizeInSamples = grainSizeInSeconds * sampleRate;
            
        }
    }
    
    
private:
    
    int sampleRate;
    int currentWriteSize;
    int currentSizeTemporary;
    int oldWriteSize;
    int oldWriteSizeTemporary;
    int maxSize;
    float* bufferL = nullptr;
    float* bufferR = nullptr;
    int writePos = 0;
    int readPos[5] = {0, 0, 0, 0, 0};
    bool grainIsPlaying[5] = {false, false, false, false, false};
    juce::Random random;
};
