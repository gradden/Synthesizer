﻿#include <iostream>
using namespace std;

#include "SoundcardHandler.h"
#include "Oscillator.h"
#include "Note.h"
#include "MidiHandler.h"
#include "Filter.h"
#include "Constants.cpp"

bool filterEnabled = false;
bool isEnvelope = false;

short oscillator_type = TRIANGLE_WAVE;
short playMode = PLAYMODE_KEYBOARD;
short soundcard = 0;

double MasterMix = 0.0;
char answer;

vector<wstring> soundcards = SoundcardHandler::getSoundcards();
SoundcardHandler* sound;
Oscillator* osc;
vector<Note> note;
MidiHandler* midihandler = new MidiHandler();
Filter* lowpass = new Filter(DEFAULT_SAMPLE_RATE, LPF_TAPS);

void loadMidi() {
	midihandler->initMidiDevice();
	bool notesArray[127] = { false };

	cout << "Play now!";
	while (1) {

		if (midihandler->getKeyPressedState() && notesArray[midihandler->getNote()] == false) {
			notesArray[midihandler->getNote()] = true;
			Note n{};
			n.noteId = midihandler->getNote();
			n.freq = BASE_MIDI_FREQUENCY * pow(pow(2.0, 1.0 / 12.0), midihandler->getNote());
			n.active = true;
			n.amplitude = osc->getAmplitude();

			note.emplace_back(n);

			osc->On(sound->getTime());
			osc->setVelocityPressure(midihandler->getVelocity());
		}

		if (!midihandler->getKeyPressedState() && notesArray[midihandler->getNote()] == true) {
			notesArray[midihandler->getNote()] = false;
			int noteId = midihandler->getNote();
			osc->Off(sound->getTime());
			
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
		cout << "\rFrequency base: " << osc->getFrequency()
			<< " Hz | Notes pressed at same time: " << note.size()
			<< " | Current Amplitude: " << osc->getAmplitude();
	}
}

void optionKeys() {
	for (int i = 1; i < 6; i++) {
		if ((GetAsyncKeyState((unsigned char)"12345"[i]) & 0x8000)) {
			oscillator_type = i;
		}
	}
}

void playOnKeyboard() {
	cout << "Play now!";

	bool pressed[MAX_KEYBOARD_BUTTON_COUNT] = { false };

	while (1) {
		for (short btnId = 0; btnId < MAX_KEYBOARD_BUTTON_COUNT; btnId++) {
			auto iterator = find_if(note.begin(), note.end(), [&btnId](const Note& obj) {return obj.noteId == btnId; });

			if (iterator == note.end()) {
				if ((GetAsyncKeyState((char)"AWSEDFTGZHUJK"[btnId]) & 0x8000)) {
					if (pressed[btnId] == false) {
						Note n{};
						n.noteId = btnId;
						n.freq = BASE_KEYBOARD_FREQUENCY * pow(2.0, (double)btnId / 12.0);
						n.active = true;
						pressed[btnId] = true;
						note.emplace_back(n);
						osc->On(sound->getTime());
					}
				}
			}
			else {
				if (!(GetAsyncKeyState((char)"AWSEDFTGZHUJK"[btnId]) & 0x8000) && pressed[btnId]) {
					pressed[btnId] = false;
					iterator->active = false;
					osc->Off(sound->getTime());
				}
			}
		}
		cout << "\rFrequency base: " << osc->getFrequency()
			<< " Hz | Notes pressed at same time: " << note.size()
			<< " | Max Amplitude: " << osc->getAmplitude();
			
		optionKeys();
		this_thread::sleep_for(5ms);
	}
}

void loadSoftware() {
	short num = 0;
	std::cout << "Soundcards: " << endl;
	for (auto soundcard : soundcards) {
		std::wcout << num << ". ";
		std::wcout << soundcard << endl;
		num++;
	}
	do {
		std::cout << "Select output: " && cin >> soundcard;
	} while (soundcard >= soundcards.size());
	sound = new SoundcardHandler(soundcards[soundcard], DEFAULT_SAMPLE_RATE, DEFAULT_OUTPUT_MODE, DEFAULT_BLOCK_COUNT, DEFAULT_BLOCK_SAMPLE);
	osc = new Oscillator();


	std::cout << "Oscillator types: \n 1 - Sine \n 2 - Triangle \n 3 - Square \n 4 - White Noise \n 5 - Sawtooth \n 6 - Two sawtooth with detune" << endl;
	std::cout << "Oscillate this: " && cin >> oscillator_type;
	if (oscillator_type == WHITE_NOISE) {
		double min = osc->getWhiteNoiseMinFreq();
		double max = osc->getWhiteNoiseMaxFreq();
		std::cout << "White Noise minimum frequency (Hz): " && cin >> min;
		std::cout << "White Noise maximum frequency (Hz): " && cin >> max;
		if (min < WHITE_NOISE_MIN_FREQ || max > WHITE_NOISE_MAX_FREQ) {
			min = WHITE_NOISE_MIN_FREQ;
			max = WHITE_NOISE_MAX_FREQ;
		}
		osc->setWhiteNoiseFreqScale(min, max);
	}

	std::cout << "Enable envelope? (Y/N): " && cin >> answer;
	if (answer == toupper('y') || answer == 'y') {
		isEnvelope = true;
		double a{}, d{}, r{};
		int mPercent{}, sPercent{};
		std::cout << "Maximum Level (1-100%): " && cin >> mPercent;
		std::cout << "Attack time (in seconds): " && cin >> a;
		std::cout << "Decay time (in seconds): " && cin >> d;
		std::cout << "Sustain level (1-100%): " && cin >> sPercent;
		std::cout << "Release time (in seconds): " && cin >> r;
		if ((mPercent > 100 || mPercent < 1) || (sPercent > 100 || sPercent < 1)) {
			std::cout << "Invalid values given. ADSR set to default values" << endl;
			osc->setEnvelope(true, ADSR_MAX_VALUE, ADSR_ATTACK_TIME, ADSR_DECAY_TIME, ADSR_SUSTAIN_LEVEL, ADSR_RELEASE_TIME);
		}
		else {
			double max = ((double)mPercent / 100);
			double sustain = ((double)sPercent / 100);
			osc->setEnvelope(true, max, a, d, sustain, r);
		}
	}
	else {
		int mPercent{};
		std::cout << "Max Level (1-100%): " && cin >> mPercent;
		double max = ((double)mPercent / 100);
		osc->setEnvelope(false, max, 0.0, 0.0, 0.0, 0.0);
		osc->setAmplitude(max);
	}

	std::cout << "Enable LPF? (Y/N): " && cin >> answer;
	if (answer == toupper('y') || answer == 'y') {
		filterEnabled = true;
		double filterFreq = LPF_CUTOFF_FREQUENCY;
		std::cout << "Cutoff frequency (Hz): " && cin >> filterFreq;
		lowpass->setCutoff(filterFreq);
	}

	std::cout << "Choose input method: \n 1 - Keyboard \n 2 - MIDI" << endl;
	std::cout << "Playmode: " && cin >> playMode;
	if (playMode == PLAYMODE_MIDI) {
		osc->setVelocity(PLAY_WITH_VELOCITY);
	}
}

double wrapper(double time) {
	double MasterMix = 0.0;
	
	for (auto n : note) {
		n.amplitude = osc->getEnvelope(sound->getTime(), n.active);
		if (n.amplitude == 0.0 && n.active == false) {
			note.erase(std::remove_if(begin(note), end(note), [&n](const Note& obj) {return obj.noteId == n.noteId; }));
		}
		MasterMix += osc->oscillate(time, n.freq, oscillator_type) * n.amplitude;
	}

	return filterEnabled ? lowpass->filter(MasterMix) : MasterMix;
}

int main()
{
	#if __linux__
	std::cout << "Unsupported OS";
	exit(0);
	#endif

	loadSoftware();
	sound->setWave(wrapper);
	std::cout << "\x1B[2J\x1B[H";

	switch (playMode)
	{
	case PLAYMODE_KEYBOARD:
		playOnKeyboard();
		break;
	case PLAYMODE_MIDI:
		loadMidi();
		break;
	}

	return 0;
}
