#include <Bela.h>
#include <libraries/Midi/Midi.h>
#include <libraries/Scope/Scope.h>
#include <stdlib.h>
#include <cmath>

Scope scope;
Midi midi;

const char* gMidiPort0 = "hw:0,0";

unsigned int gAudioChannelNum;  // number of audio channels to iterate over
unsigned int gAnalogChannelNum; // number of analog channels to iterate over

unsigned int gInterval = 20; // how often to send a MIDI message (per second)
unsigned int gCount = 0;

#define NUMINPUT 8
int inputcc[NUMINPUT] = {1, 2, 3, 4, 5, 6, 7, 8};
float inputval[NUMINPUT] = {-1., -1., -1., -1., -1., -1., -1., -1.};
enum  inputkey{input1, input2, input3, input4, input5, input6, input7, input8};

#define NUMOUTPUT 8
int outputcc[NUMINPUT] = {1, 2, 3, 4, 5, 6, 7, 8};
float outputval[NUMOUTPUT] = {-1., -1., -1., -1., -1., -1., -1., -1.};
enum  outputkey{output1, output2, output3, output4, output5, output6, output7, output8};

void on_midimessage(MidiChannelMessage message, void* arg) {
	if(arg != NULL) {
		rt_printf("Message from midi port %s ", (const char*) arg);
	}
	message.prettyPrint();

	if(message.getType() == kmmControlChange) {
		int cc = message.getDataByte(0);
		for (int ch=0; ch < gAnalogChannelNum; ch++) {
			if (cc==outputcc[ch]) {
				outputval[ch] = message.getDataByte(1) / 127.;
			}
		}
	}
}

bool setup(BelaContext *context, void *userData)
{
	// check that we have the same number of inputs and outputs
	if(context->audioInChannels != context->audioOutChannels ||
			context->analogInChannels != context-> analogOutChannels){
		printf("Different number of outputs and inputs available. Working with what we have.\n");
	}

	// use the minimum number of channels between input and output
	gAudioChannelNum = std::min(context->audioInChannels, context->audioOutChannels);
	gAnalogChannelNum = std::min(context->analogInChannels, context->analogOutChannels);
  gAnalogChannelNum = std::min(gAnalogChannelNum, NUMINPUT);
  gAnalogChannelNum = std::min(gAnalogChannelNum, NUMOUTPUT);

	// tell the scope how many channels and the sample rate
	scope.setup(gAudioChannelNum, context->audioSampleRate);
	// scope.setup(gAnalogChannelNum, context->analogSampleRate);

	midi.readFrom(gMidiPort0);
	midi.writeTo(gMidiPort0);
	midi.enableParser(true);
	midi.setParserCallback(on_midimessage, (void*) gMidiPort0);
	return true;
}

void render(BelaContext *context, void *userData)
{
	// pass audio inputs through to outputs
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		float audioval[gAudioChannelNum];
		for(unsigned int ch = 0; ch < gAudioChannelNum; ch++) {
			audioval[ch] = audioRead(context, n, ch);
			audioWrite(context, n, ch, audioval[ch]);
		}
		// show analog output values on the scope
		scope.log(audioval);
	}

	// write analog output values
	for(unsigned int n = 0; n < context->analogFrames; n++) {
		for(unsigned int ch = 0; ch < gAnalogChannelNum; ch++) {
			analogWriteOnce(context, n, ch, outputval[ch]);
		}
	}

	// read analog input values, compute average over all frames
	for(unsigned int ch = 0; ch < gAnalogChannelNum; ch++) {
		inputval[ch] = 0;
		for(unsigned int n = 0; n < context->analogFrames; n++) {
			inputval[ch] += analogRead(context, n, ch);
		}
		inputval[ch] /= context->analogFrames;
	}

	// once in a while send the analog input as MIDI control change message
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		gCount++;
		if(gCount % (int)(context->audioSampleRate/gInterval) == 0) {
			midi_byte_t statusByte = 0xB0; // control change on channel 0
			for (unsigned int ch = 0; ch < gAnalogChannelNum; ch++) {
				midi_byte_t controller = inputcc[ch];
				midi_byte_t value = inputval[ch] * 127;
				midi_byte_t bytes[3] = {statusByte, controller, value};
				midi.writeOutput(bytes, 3); // send a control change message
			}
			gCount = 0;
		}
	}
}

void cleanup(BelaContext *context, void *userData)
{

}
