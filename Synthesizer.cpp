#include <iostream>
using namespace std;

#include "SoundMaker.h"
#include "Oscillator.h"
#include "Note.h"
#include "Output.h"

vector<wstring> devices = SoundMaker<short>::ListDevices();
SoundMaker<short>* sound;
int oscillator_type = 0;
Oscillator* osc;
Output* output;
vector<Note> note;


void loadSoundMaker() {
	short num = 0;
	for (auto d : devices) {
		wcout << "Kimeneti eszközök: " << endl << num << ". " << d << endl;
		num++;
	}
	short soundcard = 0;
	cout << "Kiválasztott hangkartya: " && cin >> soundcard;
	sound = new SoundMaker<short>(devices[soundcard], 44100, 1, 8, 1024);
	osc = new Oscillator();
	

	cout << "Oscillator types: \n 1 - Sine \n 2 - Triangle \n 3 - Square \n 4 - Pink Noise" << endl;
	cout << "Oscillate this: " && cin >> oscillator_type;
}

double wrapper(double time) {
	double mix = 0.0;

	for (auto n : note) {
		mix += osc->oscillate(time, n.freq, oscillator_type) * osc->getEnvelope(time);
	}

	return mix;
}

int main()
{
	loadSoundMaker();
	sound->SetUserFunction(wrapper);

	cout << "Play now!";
	osc->setEnvelope(true, 0.5, 0.2, 0.0, 0.5, 0.3);

	output->InputProcessWithPolyphony(&note, *osc, *sound);


	

	return 0;
}
