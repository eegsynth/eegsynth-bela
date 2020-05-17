#include <Bela.h>
#include <libraries/Scope/Scope.h>

Scope scope;

unsigned int gAudioChannelNum;  // number of audio channels to iterate over
unsigned int gAnalogChannelNum; // number of analog channels to iterate over

bool setup(BelaContext *context, void *userData)
{
	bool status = true;

	// use the minimum number of channels between input and output
	gAudioChannelNum = std::min(context->audioInChannels, context->audioOutChannels);
	gAnalogChannelNum = std::min(context->analogInChannels, context->analogOutChannels);

	// tell the scope how many channels and the sample rate
	// scope.setup(gAudioChannelNum, context->audioSampleRate);
	scope.setup(gAnalogChannelNum, context->analogSampleRate);

	return status;
}

void render(BelaContext *context, void *userData)
{
	float audioval[gAudioChannelNum];
	float analogval[gAnalogChannelNum];

	// pass audio inputs through to outputs
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		for(unsigned int ch = 0; ch < gAudioChannelNum; ch++) {
			audioval[ch] = audioRead(context, n, ch);
			audioWrite(context, n, ch, audioval[ch]);
		}
		// show audio values on the scope
		// scope.log(audioval);
	}

	// pass analog inputs through to outputs
	for(unsigned int n = 0; n < context->analogFrames; n++) {
		for(unsigned int ch = 0; ch < gAnalogChannelNum; ch++) {
			analogval[ch] = analogRead(context, n, ch);
			analogWriteOnce(context, n, ch, analogval[ch]);
		}
		// show analog values on the scope
		scope.log(analogval);
	}

}

void cleanup(BelaContext *context, void *userData)
{

}
