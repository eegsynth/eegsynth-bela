#include <Bela.h>
#include <libraries/Scope/Scope.h>
#include <libraries/Midi/Midi.h>
#include <libraries/WriteFile/WriteFile.h>
#include <cmath>

#define MIDI_FEEDBACK false
#define CHECK_FREQUENCY false
#define SUBTRACT_MEAN true

// static inline bool isnan(float x) { return x != x; }
// static inline bool isinf(float x) { return !isnan(x) && isnan(x - x); }

Scope scope;
Midi midi;
WriteFile file;

const char* gMidiPort0 = "hw:1,0,0";

// according to https://github.com/BelaPlatform/Bela/wiki/Voltage-ranges
// - analog output range is 0 to 5V
// - analog input range is 0 to 4.096V
// - headphone out is 2.6Vpp AC coupled when headphone level is 0dB
// - audio input range is 1.8Vpp when PGA gain is 0dB

// to scale all channels such that an external value of -1V to +1V corresponds to an internal value of -1.0 to +1.0
// the documented scaling values do not match my observations, hence I experimentally calibrated outputs and inputs
const float AnalogOutputScale =  1./5.000;
const float AnalogInputScale  =     4.096;
const float AudioOutputScale  =  1./0.967;
const float AudioInputScale   =     0.916;

const float AnalogOutputOffset = +0.500;
const float AnalogInputOffset  = -2.048;
const float AudioOutputOffset  =  0.000;
const float AudioInputOffset   =  0.000;

unsigned int gAudioChannelNum;  // number of audio channels to iterate over
unsigned int gAnalogChannelNum; // number of analog channels to iterate over

bool gSweep = false;
float gStepSize = 10;

float gFrequency = 441; // start with a moderate frequency in the audible range
float gAmplitude = 0.5; // prevent clipping on the audio input
float gOffset = 0.0;
float gPhase = 0.0;

float gSampleRate, gInverseSampleRate;
float gBlockSize = 44100/4;
float analogDFT[] = {0, 0};
float audioDFT[] = {0, 0};
unsigned int gCount = 0; 

// use https://en.wikipedia.org/wiki/Exponential_smoothing to compute the running mean
// if the sampling time is fast compared to the time constant, then alpha = gInverseSampleRate / timeConstant
float analogMean = 0, audioMean = 0, alpha, timeConstant;

void updateFrequency() {
	// ensure that the signal frequency corresponds to an integer number of samples per block
	// this prevents spectral leakage from the large DC component that might be present
	if (CHECK_FREQUENCY && gFrequency>0)
		gFrequency = gBlockSize / (int)(gBlockSize / gFrequency);
	if (gFrequency>gSampleRate/2)
		gFrequency = gSampleRate/2;

	// update the time constant and exponential decay for the running mean
	timeConstant = 10. / gFrequency;
	alpha = gInverseSampleRate / timeConstant;

	// start a new integration window
	gCount = 0;
	analogDFT[0] = 0;
	analogDFT[1] = 0;
	audioDFT[0]  = 0;
	audioDFT[1]  = 0;
}

void midiMessageCallback(MidiChannelMessage message, void* arg){
	if (MIDI_FEEDBACK) {
		if (arg != NULL)
			rt_printf("%s ", (const char*) arg);
		message.prettyPrint();
	}
	
	if (message.getType() == kmmNoteOn) {
		int note = message.getDataByte(0);
		if (note==73) {
			gSweep = true;
		}
		else if (note==74) {
			gSweep = false;
		}
	}
	if (message.getType() == kmmControlChange) {
		int cc = message.getDataByte(0);
		if (cc==77)
			gFrequency = ((float)message.getDataByte(1)/127.) * 1000;
		else if (cc==78)
			gAmplitude = ((float)message.getDataByte(1)/127.);
		else if (cc==79)
			gOffset = ((float)message.getDataByte(1)/127.);
			
		updateFrequency();
		
		if (MIDI_FEEDBACK)
			rt_printf("gFrequency = %f, gAmplitude %f, gOffset %f\n", gFrequency, gAmplitude, gOffset);
		}
}

bool setup(BelaContext *context, void *userData)
{
	bool status = true;
	
	midi.readFrom(gMidiPort0);
	midi.enableParser(true);
	midi.setParserCallback(midiMessageCallback, (void*) gMidiPort0);
	
	if (context->analogSampleRate != context->audioSampleRate) {
		printf("ERROR: audio and analog sample rate should be identical\n");
		status = false;
	}
	
	gSampleRate = context->analogSampleRate;
	gInverseSampleRate = 1.0 / context->analogSampleRate;
	
	updateFrequency();

	// use the minimum number of channels between input and output
	gAudioChannelNum = std::min(context->audioInChannels, context->audioOutChannels);
	gAnalogChannelNum = std::min(context->analogInChannels, context->analogOutChannels);
	
	// tell the scope how many channels and the sample rate
	// scope.setup(gAudioChannelNum, context->audioSampleRate);
	scope.setup(3, context->analogSampleRate);

	file.setup("sweep.m"); //set the file name to write to
	file.setHeader("x = [\n"); //set one or more lines to be printed at the beginning of the file
	file.setFooter("];\n"); //set one or more lines to be printed at the end of the file
	file.setFormat("%8.2f, %6f, %6.2f, %6f, %6.2f\n"); // set the format that you want to use for your output. Please use %f only (with modifiers)
	file.setFileType(kText);

	return status;
}

void render(BelaContext *context, void *userData)
{
	// analytic signal, analog in, audio in
	float scopeval[3]; 
	
	// produce the analog and audio output
	for (unsigned int n = 0; n < context->analogFrames; n++) {

		gPhase += 2.0f * (float)M_PI * gFrequency * gInverseSampleRate;
		if(gPhase > M_PI)
			gPhase -= 2.0f * (float)M_PI;
		
		float cose = cosf(gPhase);
		float sine = sinf(gPhase);
		float signal = gAmplitude * cose + gOffset;
		
		for (unsigned int ch = 0; ch < gAnalogChannelNum; ch++)
			analogWriteOnce(context, n, ch, signal *AnalogOutputScale + AnalogOutputOffset );
		
		for (unsigned int ch = 0; ch < gAudioChannelNum; ch++)
			audioWrite(context, n, ch, signal * AudioOutputScale + AudioOutputOffset );
		
		float  analogVal = analogRead(context, n, 0);
		float  audioVal = audioRead(context, n, 0);
		
		scopeval[0] = signal;
		scopeval[1] = analogVal * AnalogInputScale + AnalogInputOffset;
		scopeval[2] = audioVal * AudioInputScale + AudioInputOffset;
		
		// show values on the scope
		scope.log(scopeval);

		if (SUBTRACT_MEAN && gFrequency>0) {
			// accumulate the running mean
			analogMean = alpha * scopeval[1] + (1.-alpha) * analogMean;
			audioMean  = alpha * scopeval[2] + (1.-alpha) * audioMean;

			// subtract the running mean from the measured signal
			scopeval[1] -= analogMean;
			scopeval[2] -= audioMean;
		}

		// accumulate the discrete Fourier transform
		gCount++;
		analogDFT[0] += cose*scopeval[1];
		analogDFT[1] += sine*scopeval[1];
		audioDFT[0]  += cose*scopeval[2];
		audioDFT[1]  += sine*scopeval[2];
		
		if (gCount==gBlockSize) {

			analogDFT[0] /= gAmplitude * gCount/2;
			analogDFT[1] /= gAmplitude * gCount/2;
			audioDFT[0]  /= gAmplitude * gCount/2;
			audioDFT[1]  /= gAmplitude * gCount/2;

			// there is a small delay between sending the signal out and recording it, which causes an intrinsic phase delay between the signals
			// float phaseDelay = 360. * gFrequency * (context->analogFrames/gSampleRate);

			// complete the discrete Fourier transform
			float analogAmplitude = sqrtf(analogDFT[0]*analogDFT[0] + analogDFT[1]*analogDFT[1]);
			float analogPhase = atan2f(analogDFT[1], analogDFT[0]) * 180 / (float )M_PI;
			// analogPhase -= phaseDelay;
			analogPhase -= 0.1388 * gFrequency; // this was experimentally determined for a block size of 8 samples
			analogPhase -= 180 * (int)(analogPhase/180);

			float audioAmplitude = sqrtf(audioDFT[0]*audioDFT[0] + audioDFT[1]*audioDFT[1]);
			float audioPhase = atan2f(audioDFT[1], audioDFT[0]) * 180 / (float)M_PI;
			// audioPhase -= phaseDelay;
			audioPhase += 0.1516 * gFrequency; // this was experimentally determined for a block size of 8 samples
			audioPhase -= 180 * (int)(audioPhase/180);
			
			rt_printf("%8.2f, %6f, %6.2f, %6f, %6.2f\n", gFrequency, analogAmplitude, analogPhase, audioAmplitude, audioPhase);
			
			// start a new integration window
			gCount = 0;
			analogDFT[0] = 0;
			analogDFT[1] = 0;
			audioDFT[0]  = 0;
			audioDFT[1]  = 0;

			if (gSweep) {
				// write the results to a log file
				file.log(gFrequency);
				file.log(analogAmplitude);
				file.log(analogPhase);
				file.log(audioAmplitude);
				file.log(audioPhase);

				// increment the frequency
				gFrequency += gStepSize;
				if (gFrequency >= gSampleRate/2)
					// reset the frequency
					gFrequency = 0.;
				updateFrequency();
			}
		}
	}
}

void cleanup(BelaContext *context, void *userData)
{

}
