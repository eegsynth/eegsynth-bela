# frequency-response

The purpose of this project is to measure the linear transfer of an
amplifier and determine its frequency response. A sine-wave signal is
created and send to the analog output and to the audio autput. At the
same time, the analog input and the audio input are recorded. Using a
discrete fourier transform, the amplitude and phase characteristics of
the external system between the output and input are computed.

A Launchcontrol XL MIDI interface can be used to select the frequency,
amplitude and DC offset, and to start and stop a frequency sweep.

This project includes an EEGsynth patch to emulate a Launchcontrol XL
interface. It also includes some MATLAB code to read the data written
during the sweep from file and to make a Bode plot of the frequency
response.
