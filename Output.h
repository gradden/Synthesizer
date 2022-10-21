#pragma once
#include<iostream>
#include <string>
#include<vector>
#include <WinUser.h>
#include "Note.h"
#include "Oscillator.h"
#include "SoundMaker.h"

using namespace std;
typedef bool(*lambda)(Note const& note);

class Output {

private:
	string method;

public:

	void static InputProcessWithPolyphony(vector<Note>* note, Oscillator& osc, SoundMaker<short>& sound) {
		while (1) {
			for (int i = 0; i < 16; i++) {

				auto iterator = find_if(note->begin(), note->end(), [&i](const Note& obj) {return obj.noteId == i; });
				
				if (iterator == note->end()) {
					if (GetAsyncKeyState((unsigned char)"AWSEDFTGZHUJK"[i]) & 0x8000) {
						Note n{};
						n.noteId = i;
						n.freq = 440.0 * pow(pow(2.0, 1.0 / 12.0), i);
						n.active = true;
						osc.On(sound.GetTime());
						note->push_back(n);
					}

				}
				else {
				
					if (!(GetAsyncKeyState((unsigned char)"AWSEDFTGZHUJK"[i]) & 0x8000)) {
						if (iterator != note->end()) {
							if (osc.isEnvelopeEnabled()) {
								osc.Off(sound.GetTime());
								while (osc.getAmplitude() > 0.01) {}
							}	
							for (vector<Note>::iterator it = note->begin(); it != note->end(); ++it) {
								if (i == it->noteId) {
									note->erase(it);
									break;
								}
							}
						}
					}
				}

				for (int i = 0; i < note->size(); i++) {
					wcout << " " << note->at(i).noteId;
				}
				cout << endl;
				this_thread::sleep_for(5ms);
			}
			
		}
	}

};