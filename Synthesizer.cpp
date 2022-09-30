#include <iostream>
using namespace std;

#include "SoundMaker.h"

atomic<double> dFrequencyOutput = 0.0;
double dOctaveBaseFrequency = 440.0;
double d12thRootOf2 = pow(2.0, 1.0 / 12.0);

double MakeNoise(double dTime)
{
	double dOutput = sin(dFrequencyOutput * 2.0 * 3.14159 * dTime);
	return dOutput * 0.5; // Master Volume
}

int main()
{
	vector<wstring> devices = SoundMaker<short>::ListDevices();
	for (auto d : devices) wcout << "Found Output Device: " << d << endl;
	int soundcard = 0;
	cout << "Kiválasztott hangkartya: ";
	cin >> soundcard;

	
	SoundMaker<short> sound(devices[soundcard], 44100, 1, 8, 512);

	sound.SetUserFunction(MakeNoise);

	return 0;
}
