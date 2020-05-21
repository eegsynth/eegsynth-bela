#include <Bela.h>
#include <libraries/Scope/Scope.h>
#include <libraries/Midi/Midi.h>
#include <cmath>

Scope scope;

unsigned int gAudioChannelNum;  // number of audio channels to iterate over
unsigned int gAnalogChannelNum; // number of analog channels to iterate over

// according to https://github.com/BelaPlatform/Bela/wiki/Voltage-ranges
// - analog output range is 0 to 5V
// - analog input range is 0 to 4.096V  
// - headphone out is 2.6Vpp AC coupled when headphone level is 0dB
// - audio input range is 1.8Vpp when PGA gain is 0dB

// try to scale all channels such that an external value of -1V to +1V corresponds to an internal value of -1.0 to +1.0
// the scaling values above don't seem to match my observations, hence I experimentally calibrated the audio output and input 
#define AnalogOutputScale 1./5.000
#define AnalogInputScale     4.096
#define AudioOutputScale 1./(0.967)
#define AudioInputScale      0.916

# define AnalogOutputOffset +0.500
# define AnalogInputOffset  -2.048
# define AudioOutputOffset   0.0
# define AudioInputOffset    0.0

float gFrequency = 30.0;
float gAmplitude = 1.0;  
float gOffset = 0.0;  
float gPhase = 0.0;
float gInverseSampleRate;


void midiMessageCallback(MidiChannelMessage message, void* arg){
	if(arg != NULL){
		rt_printf("%s ", (const char*) arg);
	}
	message.prettyPrint();
	if(message.getType() == kmmControlChange){
        int cc = message.getDataByte(0);
		if (cc==77)
			gFrequency = ((float)message.getDataByte(1)/127.) * 440. * 2 * 2 * 2 * 2 * 2;
		if (cc==78)
			gAmplitude = ((float)message.getDataByte(1)/127.);
		if (cc==79)
			gOffset = ((float)message.getDataByte(1)/127.);
		rt_printf("gFrequency = %f, gAmplitude %f, gOffset %f\n", gFrequency, gAmplitude, gOffset);
	}
}

Midi midi;

const char* gMidiPort0 = "hw:1,0,0";

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

		gInverseSampleRate = 1.0 / context->analogSampleRate;

        // use the minimum number of channels between input and output
        gAudioChannelNum = std::min(context->audioInChannels, context->audioOutChannels);
        gAnalogChannelNum = std::min(context->analogInChannels, context->analogOutChannels);

        // tell the scope how many channels and the sample rate
        // scope.setup(gAudioChannelNum, context->audioSampleRate);
        scope.setup(3, context->analogSampleRate);

        return status;
}

void render(BelaContext *context, void *userData)
{
        float scopeval[3];		// analog out, analog in, audio in

        float analogval[gAnalogChannelNum];
        float audioval[gAudioChannelNum];

		// read the audio input
        for (unsigned int n = 0; n < context->audioFrames; n++) {
            for(unsigned int ch = 0; ch < gAudioChannelNum; ch++) {
                // audioval[ch] = audioRead(context, n, ch);
            }
        }
	
		// read the analog input
        for (unsigned int n = 0; n < context->analogFrames; n++) {
            for(unsigned int ch = 0; ch < gAnalogChannelNum; ch++) {
                // analogval[ch] = analogRead(context, n, ch);
            }
        }

		// produce the analog and audio output
        for (unsigned int n = 0; n < context->analogFrames; n++) {

			gPhase += 2.0f * (float)M_PI * gFrequency * gInverseSampleRate;
			if(gPhase > M_PI)
				gPhase -= 2.0f * (float)M_PI;

			float signal = gAmplitude * sinf(gPhase) + gOffset;

            for(unsigned int ch = 0; ch < gAnalogChannelNum; ch++)
				analogWriteOnce(context, n, ch, signal *AnalogOutputScale + AnalogOutputOffset );
				
            for(unsigned int ch = 0; ch < gAudioChannelNum; ch++)
		        audioWrite(context, n, ch, signal * AudioOutputScale + AudioOutputOffset );

			unsigned int ch = 0;

            scopeval[0] = signal;
            float tmp1 = analogRead(context, n, ch);
            float tmp2 = audioRead(context, n, ch);
			scopeval[1] = tmp1 * AnalogInputScale + AnalogInputOffset;
			scopeval[2] = tmp2 * AudioInputScale + AudioInputOffset;

    		// show values on the scope
            scope.log(scopeval);
        }

}

void cleanup(BelaContext *context, void *userData)
{

}