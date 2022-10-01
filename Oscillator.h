#pragma once
#define _USE_MATH_DEFINES

#include<math.h>

class Oscillator {
private:
	double amplitude;
	double frequency;
	int osc_type;

	double SineWave(double frequency, double time) {
		return sin(frequency * 2.0 * M_PI * time);
	}

	double TriangleWave(double frequency, double time) {
		return asin(SineWave(frequency, time));
	}

	double SquareWave(double frequency, double time) {
		return SineWave(frequency, time) > 0.0 ? 1.0 : 0.0;
	}

	double PinkNoise(double time) {
		return SineWave((double)(rand() % 10000 + 100), time);
	}

public:
	Oscillator() {
		this->amplitude = 0.0;
		this->frequency = 440.0;
		this->osc_type = 1;
	}

	void setAmplitude(double amp) {
		this->amplitude = amp;
	}

	void setFrequency(double hz) {
		this->frequency = hz;
	}
	
	double getFrequency() {
		return this->frequency;
	}

	double oscillate(double time, double frequency, int osc) {
		switch (osc)
		{
		default:
			return 0.0;
			break;
		case 1:
			return SineWave(frequency, time) * this->amplitude;
			break;
		case 2:
			return TriangleWave(frequency, time) * this->amplitude;
			break;
		case 3:
			return SquareWave(frequency, time) * this->amplitude;
			break;
		case 4:
			return PinkNoise(time) * this->amplitude;
			break;
		}
	}
};
