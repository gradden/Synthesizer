﻿#include <iostream>
using namespace std;

#include "SoundMaker.h"
#include "Oscillator.h"
#include "Note.h"
#include "Output.h"
#include "MidiHandler.h"
#include "Filter.h"

const short SINE_WAVE = 1;
const short TRIANGLE_WAVE = 2;
const short SQUARE_WAVE = 3;
const short WHITE_NOISE = 4;
const short SAWTOOTH_WAVE = 5;

const short PLAYMODE_KEYBOARD = 1;
const short PLAYMODE_MIDI = 2;

const int DEFAULT_SAMPLE_RATE = 44100;
const short DEFAULT_OUTPUT_MODE = 1;
const short DEFAULT_BLOCK_COUNT = 8;
const int DEFAULT_BLOCK_SAMPLE = 512;

const double ADSR_MAX_VALUE = 0.1;
const double ADSR_ATTACK_TIME = 0.2;
const double ADSR_DECAY_TIME = 0.1;
const double ADSR_SUSTAIN_LEVEL = 0.1;
const double ADSR_RELEASE_TIME = 0.1;

const double BASE_KEYBOARD_FREQUENCY = 440.0;
const double BASE_MIDI_FREQUENCY = 8.1758;

const int LPF_TAPS = 50;
const double LPF_CUTOFF_FREQUENCY = 150.0;

const int WHITE_NOISE_MIN_FREQ = 10;
const int WHITE_NOISE_MAX_FREQ = 20000;

bool filterEnabled = false;
bool isEnvelope = false;

short oscillator_type = TRIANGLE_WAVE;
short playMode = DEFAULT_OUTPUT_MODE;
short soundcard = 0;

double MasterMix = 0.0;

char answer;

vector<wstring> devices = SoundMaker<short>::ListDevices();
SoundMaker<short>* sound;
Oscillator* osc;
Output* output;
vector<Note> note;
MidiHandler* midihandler = new MidiHandler();
mutex muxNotes;
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

			osc->On(sound->GetTime());
			osc->setVelocityPressure(midihandler->getVelocity());
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

	bool pressed[16] = { false };

	while (1) {
		for (int i = 0; i < 16; i++) {

			auto iterator = find_if(note.begin(), note.end(), [&i](const Note& obj) {return obj.noteId == i; });

			if (iterator == note.end()) {
				if ((GetAsyncKeyState((unsigned char)"AWSEDFTGZHUJK"[i]) & 0x8000)) {
					if (pressed[i] == false) {
						Note n{};
						n.noteId = i;
						n.freq = BASE_KEYBOARD_FREQUENCY * pow(2.0, (double)i / 12.0);
						n.active = true;
						n.amplitude = osc->getAmplitude();

						pressed[i] = true;
						note.emplace_back(n);

						osc->On(sound->GetTime());
					}
				}
			}
			else {
				cout << "\r";
				if (!(GetAsyncKeyState((unsigned char)"AWSEDFTGZHUJK\xbcL\xbe\xbf"[i]) & 0x8000) && pressed[i]) {
					osc->Off(sound->GetTime());
					
					pressed[i] = false;
					iterator->active = false;
					note.erase(iterator);
				}	
			}			
		}
		cout << "\rFrequency base: " << osc->getFrequency()
			<< " Hz | Notes pressed at same time: " << note.size()
			<< " | Current Amplitude: " << osc->getAmplitude();
			
		optionKeys();
		this_thread::sleep_for(5ms);
	}
}

void loadSoundMaker() {
	short num = 0;
	for (auto d : devices) {
		wcout << "Kimeneti eszközök: " << endl << num << ". " << d << endl;
		num++;
	}
	do {
		std::cout << "Kiválasztott hangkartya: " && cin >> soundcard;
	} while (soundcard >= devices.size());
	sound = new SoundMaker<short>(devices[soundcard], DEFAULT_SAMPLE_RATE, DEFAULT_OUTPUT_MODE, DEFAULT_BLOCK_COUNT, DEFAULT_BLOCK_SAMPLE);
	osc = new Oscillator();


	std::cout << "Oscillator types: \n 1 - Sine \n 2 - Triangle \n 3 - Square \n 4 - White Noise \n 5 - Sawtooth" << endl;
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
}

double wrapper(double time) {
	unique_lock<mutex> lm(muxNotes);
	double MasterMix = 0.0;

	for (auto n : note) {
		MasterMix += osc->oscillate(time, n.freq, oscillator_type);
	}

	return filterEnabled ? lowpass->filter(MasterMix) : MasterMix;
}

int main()
{
	#if __linux__
	std::cout << "Unsupported OS";
	exit(0);
	#endif

	loadSoundMaker();
	sound->SetUserFunction(wrapper);
	osc->setVelocity(true);
	std::cout << "\x1B[2J\x1B[H";

	if (playMode == PLAYMODE_KEYBOARD) {
		thread KeyboardProcess = thread(playOnKeyboard);
		KeyboardProcess.join();
	}
	else if (playMode == PLAYMODE_MIDI) {
		thread MidiProcess = thread(loadMidi);
		MidiProcess.join();
	}

	return 0;
}
