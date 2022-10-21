#include <iostream>
using namespace std;

#include "SoundMaker.h"
#include "Oscillator.h"
#include "Note.h"
#include "Output.h"
#include "MidiHandler.h"

vector<wstring> devices = SoundMaker<short>::ListDevices();
SoundMaker<short>* sound;
int oscillator_type = 5;
Oscillator* osc;
Output* output;
vector<Note> note;
MidiHandler* midihandler = new MidiHandler();

void loadMidi() {
	midihandler->listMidiDevices();
}


void loadSoundMaker() {
	short num = 0;
	for (auto d : devices) {
		wcout << "Kimeneti eszközök: " << endl << num << ". " << d << endl;
		num++;
	}
	short soundcard = 0;
	//cout << "Kiválasztott hangkartya: " && cin >> soundcard;
	sound = new SoundMaker<short>(devices[soundcard], 44100, 1, 8, 1024);
	osc = new Oscillator();


	cout << "Oscillator types: \n 1 - Sine \n 2 - Triangle \n 3 - Square \n 4 - Pink Noise" << endl;
	//cout << "Oscillate this: " && cin >> oscillator_type;

	loadMidi();
}

double wrapper(double time) {
	double mix = 0.0;

	for (auto n : note) {
		mix += osc->oscillate(time, n.freq, oscillator_type);
	}
	//cout << "Value: " << mix << " || Time: " << time << endl;
	return mix;
}

int main()
{
	loadSoundMaker();
	sound->SetUserFunction(wrapper);

	cout << "Play now!";
	osc->setEnvelope(true, 0.1, 0.2, 0.1, 0.2, 0.2);

	//playing.join();

	while (1) {


		for (int i = 0; i < 16; i++) {

			auto iterator = find_if(note.begin(), note.end(), [&i](const Note& obj) {return obj.noteId == i; });

			if (iterator == note.end()) {
				if (GetAsyncKeyState((unsigned char)"AWSEDFTGZHUJK"[i]) & 0x8000) {
					Note n{};
					n.noteId = i;
					n.freq = 440.0 * pow(pow(2.0, 1.0 / 12.0), i);
					n.active = true;
					n.amplitude = osc->getAmplitude();

					note.emplace_back(n);

					osc->On(sound->GetTime());
				}
			}
			else {
				//cout << iterator->noteId << endl;
				if (iterator->active) {
					if (!(GetAsyncKeyState((unsigned char)"AWSEDFTGZHUJK\xbcL\xbe\xbf"[i]) & 0x8000)) {
						osc->Off(sound->GetTime());
						while (osc->getAmplitude() >= 0.01) {
							iterator->amplitude = osc->getAmplitude();
						}
						iterator->active = false;
					}
				}
				else {
					note.erase(iterator);
				}
			}
		}
		this_thread::sleep_for(5ms);
	}

	return 0;
}
