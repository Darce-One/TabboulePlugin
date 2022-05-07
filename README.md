# TabboulePlugin

The aim is a Granulator plugin which takes in the input from the DAW and plays
back the granulated audio, along with some synths.

The way this will work is by having a delay line into which the new samples
are written into, and from which the grains will be able to playback.
The Plugin will also analyse the incoming signal and chop it in pieces to be
frequency analysed and then play back a triangular wave at the base frequency.

The user controllable parameters so far are:
* Bowl size: Delay line size
* Parsley chop: grain size
* Parsley type: envelope pattern
* Mama's Hands: grain randomiser gain
* Onion: Number and phase of grains.
* Bourghol: Chance of omitted grains.
* Spices: Panning randomness.
* Oil: Reverb
* Lemon: High pass filter on grains
* Tomato size: Volume of synths
* Tomato abundance: Tolerance level on FFTSynths
* Tomato shape: Synth envelope
* Fresh mint: Synth Quantize to 12 tone!

There is a lot of room for progress, the two main and simplest
to implement at this stage are:
* A 'Tomato type' parameter, allowing for smooth transition between a sine,
triangle and sawtooth oscillator.
* A volume control for the parsley, and a wider range of volume for the Synths.

## Inspiration:

I'm not entirely sure where the idea for this plugin came about. I first
thought of it while brainstorming for the drone project.
I remember hearing of the Milk Box Compressor guitar pedal, and I really liked
the concept. The concept of granulation was fresh in my mind too at the time,
having tried out and really liked the plugin 'Emergence' by Daniel Gergely.
I realised then the potential for granulation, and wanted to take it a step in
a different direction: adding synths.
I know that the synths in the tabbouleh plugin are as basic as they get
(Triangle wave + triangle envelope), but it is a concept I plan on expanding
on over time. The infrastructure is laid out in such a way that more
information than just pitch and a crude volume can be extracted from the
grains running into the FFTSynths, and they can all be mapped to many more
parameters, such as FM synthesis to say the least.

## Known issues:

When running in the Plugin Host, changing the sampleRate results in a tuning
error. That is because the prepare to play is not run at the start when
changing the sample rate in that software. 
