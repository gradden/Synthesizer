#include <iostream>
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

vector<wstring> devices = SoundcardHandler::getSoundcards();
SoundcardHandler* sound;
Oscillator* osc;
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

			osc->On(sound->getTime());
			osc->setVelocityPressure(midihandler->getVelocity());
		}

		if (!midihandler->getKeyPressedState() && notesArray[midihandler->getNote()] == true) {
			notesArray[midihandler->getNote()] = false;
			int noteId = midihandler->getNote();
			auto it = find_if(note.begin(), note.end(), [&noteId](const Note& obj) {return obj.active && obj.noteId == noteId; });
			osc->Off(sound->getTime());

			if (it->amplitude <= 0.01) {
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

						pressed[i] = true;
						note.emplace_back(n);

						osc->On(sound->getTime());
					}
				}
			}
			else {
				cout << "\r";
				if (!(GetAsyncKeyState((unsigned char)"AWSEDFTGZHUJK\xbcL\xbe\xbf"[i]) & 0x8000) && pressed[i]) {
					pressed[i] = false;
					iterator->active = false;
					osc->Off(sound->getTime());
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
	std::cout << "Soundcards: " << endl;
	for (auto d : devices) {
		std::wcout << num << ". ";
		std::wcout << d << endl;
		num++;
	}
	do {
		std::cout << "Select output: " && cin >> soundcard;
	} while (soundcard >= devices.size());
	sound = new SoundcardHandler(devices[soundcard], DEFAULT_SAMPLE_RATE, DEFAULT_OUTPUT_MODE, DEFAULT_BLOCK_COUNT, DEFAULT_BLOCK_SAMPLE);
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
	unique_lock<mutex> lm(muxNotes);
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

	loadSoundMaker();
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
