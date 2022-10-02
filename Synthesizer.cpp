#include <iostream>
using namespace std;

#include "SoundMaker.h"
#include "Oscillator.h"

vector<wstring> devices = SoundMaker<short>::ListDevices();
SoundMaker<short>* sound;
int oscillator_type = 0;
Oscillator* osc;
bool pressed;
int key = -1;

void loadSoundMaker() {

	for (auto d : devices) {
		short num = 0;
		wcout << "Kimeneti eszközök: " << endl << num << ". " << d << endl;
		num++;
	}
	short soundcard = 0;
	cout << "Kiválasztott hangkartya: " && cin >> soundcard;
	sound = new SoundMaker<short>(devices[soundcard], 44100, 1, 8, 1024);
	osc = new Oscillator();
	

	cout << "Oscillator types: \n 1 - Sine \n 2 - Triangle \n 3 - Square \n 4 - Pink Noise" << endl;
	cout << "Oszcillate this: " && cin >> oscillator_type;
}

double wrapper(double time) {
	return (osc->oscillate(time, osc->getFrequency(), oscillator_type));
}

int main()
{
	loadSoundMaker();
	sound->SetUserFunction(wrapper);

	cout << "Play now!";
	osc->setEnvelope(true, 0.5, 0.1, 0.0, 0.5, 0.3);
	while (1) {
		
		pressed = false;
		for (int i = 0; i < 16; i++) {
			if (GetAsyncKeyState((unsigned char)"AWSEDFTGZHUJK"[i]) & 0x8000) {
				if (key != i) {
					osc->setFrequency(440.0 * pow(pow(2.0, 1.0 / 12.0), i));
					osc->On(sound->GetTime());
					//osc->setAmplitude(0.3, sound->GetTime());
					key = i;
				}
				pressed = true;
			}
		}
		if (!pressed) {
			if (key != -1) {
				osc->Off(sound->GetTime());
				key = -1;
			}
		}
	}

	return 0;
}
