# filter-transfer-bode-plot

The purpose of this project is to measure the linear transfer of an amplifier 
and determine its filter characteristics. A sine-wave signal is created and 
send to the analog output and to the audio autput. At the same time, the analog 
input and the audio input are recorded. Using a discrete fourier transform, the 
amplitude and phase characteristics of the external system between the output and 
input are computed.

TODO: 
- implement DTF
- implement sweep
- print transfer characteristics in the Bela console
