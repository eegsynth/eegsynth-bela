#include <Bela.h>
#include <libraries/OscSender/OscSender.h>
#include <libraries/OscReceiver/OscReceiver.h>
#include <libraries/Scope/Scope.h>

Scope scope;

OscReceiver oscReceiver;
OscSender oscSender;
int localPort = 7562;
int remotePort = 7563;
const char* remoteIp = "192.168.6.1";

int gAudioChannelNum;  // number of audio channels to iterate over
int gAnalogChannelNum; // number of analog channels to iterate over
float gInterval = 0.05;
int gCount = 0;

#define NUMINPUT 8
char *inputstr[NUMINPUT] = {"/bela/input1", "/bela/input2", "/bela/input3", "/bela/input4", "/bela/input5", "/bela/input6", "/bela/input7", "/bela/input8"};
float inputval[NUMINPUT] = {-1., -1., -1., -1., -1., -1., -1., -1.};
enum  inputkey{input1, input2, input3, input4, input5, input6, input7, input8};

#define NUMOUTPUT 8
char *outputstr[NUMOUTPUT] = {"/bela/output1", "/bela/output2", "/bela/output3", "/bela/output4", "/bela/output5", "/bela/output6", "/bela/output7", "/bela/output8"};
float outputval[NUMOUTPUT] = {-1., -1., -1., -1., -1., -1., -1., -1.};
enum  outputkey{output1, output2, output3, output4, output5, output6, output7, output8};

void on_receive(oscpkt::Message* msg, void* arg)
{
	for (unsigned int ch = 0; ch < NUMINPUT; ch++) {
		if(msg->match(outputstr[ch])){
			msg->match(outputstr[ch]).popFloat(outputval[ch]).isOkNoMoreArgs();
			printf("%s = %f\n", outputstr[ch], outputval[ch]);
		}
	}
}

bool setup(BelaContext *context, void *userData)
{
	bool status = true;

	// Check that we have the same number of inputs and outputs.
	if(context->audioInChannels != context->audioOutChannels ||
			context->analogInChannels != context-> analogOutChannels){
		printf("Different number of outputs and inputs available. Working with what we have.\n");
	}

	// use the minimum number of channels between input and output
	gAudioChannelNum = std::min(context->audioInChannels, context->audioOutChannels);
	gAnalogChannelNum = std::min(context->analogInChannels, context->analogOutChannels);

	// tell the scope how many channels and the sample rate
	scope.setup(gAudioChannelNum, context->audioSampleRate);
	// scope.setup(gAnalogChannelNum, context->analogSampleRate);

	oscReceiver.setup(localPort, on_receive);
	oscSender.setup(remotePort, remoteIp);
	oscSender.newMessage("/bela/status").add(status).send();
	return status;
}

void render(BelaContext *context, void *userData)
{
	// pass audio inputs through to outputs
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		float audioval[gAudioChannelNum];
		for(unsigned int ch = 0; ch < gAudioChannelNum; ch++){
			audioval[ch] = audioRead(context, n, ch) * inputval[ch];
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

	for(unsigned int n = 0; n < context->audioFrames; n++) {
		gCount++;

		if(gCount % (int)(context->audioSampleRate*gInterval) == 0) {
			for(unsigned int ch = 0; ch < NUMOUTPUT; ch++) {
				oscSender.newMessage(inputstr[ch]).add(inputval[ch]).send();
			}
		}
	}
}

void cleanup(BelaContext *context, void *userData)
{

}
