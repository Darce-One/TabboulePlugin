# TabboulePlugin

The aim is a Granulator plugin which takes in the input from the DAW and plays
back the granulated audio.

The way this will work is by having a delay line into which the new samples
are written into, and from which the grains will be able to playback.
The Plugin will also analyse the incoming signal and chop it in pieces to be
frequency analysed and then play back a triangular wave at the base frequency.

The user controllable parameters will be:
* Bowl size: Delay line size
* Parsley chop: grain size
* Parsley type: envelope pattern
* Mama's Hands: grain randomiser gain
* Oil: Reverb
* Lemon: High pass filter on grains
* Tomato size: Volume of synths
* Tomato Amount: frequency of FFT and note change
* Tomato cut: Synth envelope
* Bulgur: Chance of omitted samples
* Spices: number of individual grains 1-4?
* Onion: Number and phase of grains.
