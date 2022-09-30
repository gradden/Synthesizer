#pragma once
#define _USE_MATH_DEFINES

#include<math.h>

vector<wstring> devices = SoundMaker<short>::ListDevices();
SoundMaker<short> *sound;
int oscillator_type = 0;

void loadSoundMaker() {
	
	for (auto d : devices) wcout << "Found Output Device: " << d << endl;
	int soundcard = 0;
	cout << "Kiválasztott hangkartya: " && cin >> soundcard;
	sound = new SoundMaker<short>(devices[soundcard], 44100, 1, 8, 512);

	cout << "Oszcillátor: " && cin >> oscillator_type;
}

double SineWave(double frequency, double time) {
	return sin(frequency * 2.0 * M_PI * time);
}

double TriangleWave(double frequency, double time) {
	return asin(SineWave(frequency, time));
}

double oscillator(double time, double frequency, int osc) {
	switch (osc)
	{
	default:
		return 0.0;
		break;
	case 1:
		return SineWave(frequency, time);
		break;
	case 2:
		return TriangleWave(frequency, time);
		break;
	}
}

double wrapper(double time) {
	return oscillator(time, 440, oscillator_type) * 0.3;
}

