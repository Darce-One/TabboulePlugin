/*
  ==============================================================================

    GrainBuffer.h
    Created: 13 Apr 2022 11:05:09pm
    Author:  B162025

  ==============================================================================
*/

#pragma once

class GrainBuffer
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
    ~GrainBuffer()
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
        writePos++;
        
        if (writePos >= currentWriteSize || writePos >= maxSize)
        {
            writePos = 0;
            // Must insure that the current size used here only changes when the
            // write position is reset to ensure no gaps in playback
            maxReadPos = currentWriteSize;
            currentWriteSize = currentSizeTemporary;

        }
        
        bufferL[writePos] = inputSampleL;
        bufferR[writePos] = inputSampleR;
        
    }
    
    /**
     Reads the LEFT sample at the reading position of the specified grain.
     
     @param index The reading position index [0-4]
     @return sample value at reading position
     */
    float readValL(int index)
    {
        return bufferL[readPos];
    }
    
    /**
     Reads the RIGHT sample at the reading position of the specified grain.
     
     @param index The reading position index [0-4]
     @return sample value at reading position
     */
    float readValR(int index)
    {
        return bufferR[readPos];
    }
    
    float getMaxReadPos()
    {
        return maxReadPos;
    }
    
    
private:
    
    int sampleRate;
    int currentWriteSize;
    int currentSizeTemporary;
    int oldWriteSize;
    int maxReadPos;
    int maxSize;
    float* bufferL = nullptr;
    float* bufferR = nullptr;
    int writePos = 0;
    int readPos = 0;
};


