#include <iostream>
using namespace std;

#include "SoundMaker.h"
#include "Oscillator.h"
#include "Note.h"
#include "Output.h"
#include "MidiHandler.h"

vector<wstring> devices = SoundMaker<short>::ListDevices();
SoundMaker<short>* sound;
short oscillator_type = 2;
short soundcard = 0;
short playMode = 2;
double MasterMix = 0.0;
Oscillator* osc;
Output* output;
vector<Note> note;
MidiHandler* midihandler = new MidiHandler();
mutex muxNotes;

void loadMidi() {
	midihandler->initMidiDevice();
	bool notesArray[127] = { false };

	cout << "Play now!";
	while (1) {

		if (midihandler->getKeyPressedState() && notesArray[midihandler->getNote()] == false) {
			notesArray[midihandler->getNote()] = true;
			Note n{};
			n.noteId = midihandler->getNote();
			n.freq = 8.1758 * pow(pow(2.0, 1.0 / 12.0), midihandler->getNote());
			n.active = true;
			n.amplitude = osc->getAmplitude();

			note.emplace_back(n);

			osc->On(sound->GetTime());
		}

		if (!midihandler->getKeyPressedState() && notesArray[midihandler->getNote()] == true) {
			notesArray[midihandler->getNote()] = false;
			int noteId = midihandler->getNote();
			auto it = find_if(note.begin(), note.end(), [&noteId](const Note& obj) {return obj.active && obj.noteId == noteId; });
			osc->Off(sound->GetTime());
			while (it->amplitude >= 0.01) {
				it->amplitude = osc->getAmplitude();
			}

			if (it != note.end()) {
				note.erase(it);
			}

			bool allEmpty = true;
			for (auto a : notesArray) {
				if (a != false) {
					allEmpty = false;
				}
			}
			if (allEmpty) {
				note.clear();
			}
			
		}

		cout << "\r" << note.size();
	}
}



void playOnKeyboard() {
	cout << "Play now!";
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
}

void loadSoundMaker() {
	short num = 0;
	for (auto d : devices) {
		wcout << "Kimeneti eszközök: " << endl << num << ". " << d << endl;
		num++;
	}
	cout << "Kiválasztott hangkartya: " && cin >> soundcard;
	sound = new SoundMaker<short>(devices[soundcard], 44100, 1, 8, 1024);
	osc = new Oscillator();


	cout << "Oscillator types: \n 1 - Sine \n 2 - Triangle \n 3 - Square \n 4 - Pink Noise \n 5 - Sawtooth" << endl;
	cout << "Oscillate this: " && cin >> oscillator_type;

	cout << "Choose input method: \n 1 - Keyboard \n 2 - MIDI" << endl;
	cout << "Playmode: " && cin >> playMode;
}

double wrapper(double time) {
	unique_lock<mutex> lm(muxNotes);
	MasterMix = 0.0;

	for (auto n : note) {
		MasterMix += osc->oscillate(time, n.freq, oscillator_type);
	}
	//cout << "Value: " << mix << " || Time: " << time << endl;
	return MasterMix;
}

int main()
{
	loadSoundMaker();
	sound->SetUserFunction(wrapper);
	//osc->setEnvelope(true, 0.1, 0.2, 0.1, 0.2, 0.2);
	osc->setEnvelope(true, 0.1, 0.001, 0.3, 0.0, 0.2);
	
	

	if (playMode == 1) {
		//thread KeyboardProcess = thread(playOnKeyboard);
		//KeyboardProcess.join();
		playOnKeyboard();
	}
	else if (playMode == 2) {
		thread MidiProcess = thread(loadMidi);
		MidiProcess.join();
	}

	return 0;
}
